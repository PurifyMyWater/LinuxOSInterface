#include "LinuxTimer.h"
#include "OSInterface_Log.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <mutex>

#define CLOCKID CLOCK_REALTIME
#define TIMER_SIG SIGRTMIN

void LinuxTimer::initializeSignalSystem()
{
    struct sigaction sa{};
    sa.sa_flags     = SA_SIGINFO;
    sa.sa_sigaction = signalHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(TIMER_SIG, &sa, nullptr))
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to setup sigaction: %s", strerror(errno));
        exit(errno);
    }
}

void LinuxTimer::signalHandler(int sig, siginfo_t* si, void* uc)
{
    if (si == nullptr || si->si_signo != TIMER_SIG)
    {
        return; // A missdelivered signal has been received, ignore it
    }

    if (LinuxTimer* timer = static_cast<LinuxTimer*>(si->si_value.sival_ptr);
        timer != nullptr && timer->callbackFunction != nullptr)
    {
        timer->callbackFunction(timer->callbackArg);
    }
}

LinuxTimer::LinuxTimer(uint32_t period, OSInterface_Timer::Mode mode, OSInterfaceProcess callback, void* callbackArg,
                       const char* timerName)
{
    static std::once_flag sigSetupFlag;
    std::call_once(sigSetupFlag, [this]() { initializeSignalSystem(); });

    this->name = strdup(timerName);

    this->mode = mode;

    this->callbackFunction = callback;
    this->callbackArg      = callbackArg;

    this->timerSpec.it_value.tv_sec  = period / 1000;
    this->timerSpec.it_value.tv_nsec = (period % 1000) * 1000000;

    if (mode == PERIODIC)
    {
        this->timerSpec.it_interval.tv_sec  = this->timerSpec.it_value.tv_sec;
        this->timerSpec.it_interval.tv_nsec = this->timerSpec.it_value.tv_nsec;
    }

    this->sev.sigev_notify          = SIGEV_SIGNAL;
    this->sev.sigev_signo           = TIMER_SIG;
    this->sev.sigev_value.sival_ptr = this;

    if (timer_create(CLOCKID, &this->sev, &this->timerId) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to create timer '%s': %s", timerName ? timerName : "unknown",
                            strerror(errno));
        exit(errno);
    }
}

LinuxTimer::~LinuxTimer()
{
    if (name != nullptr)
    {
        free(name);
    }
    if (!stop() || timer_delete(timerId) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to disarm or destroy timer: %s", strerror(errno));
        exit(errno);
    }
}

bool LinuxTimer::start()
{
    return timer_settime(timerId, 0, &timerSpec, nullptr) == 0;
}

bool LinuxTimer::startFromISR()
{
    return start();
}

bool LinuxTimer::stop()
{
    itimerspec stopTimerSpec{};
    return timer_settime(timerId, 0, &stopTimerSpec, nullptr) == 0;
}

bool LinuxTimer::stopFromISR()
{
    return stop();
}

[[nodiscard]] bool LinuxTimer::isRunning() const
{
    itimerspec currentSpec{};
    if (timer_gettime(timerId, &currentSpec) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to get timer information: %s", strerror(errno));
        exit(errno);
    }
    return currentSpec.it_value.tv_sec != 0 || currentSpec.it_value.tv_nsec != 0;
}

bool LinuxTimer::setPeriod(uint32_t newPeriod_ms)
{
    timerSpec.it_value.tv_sec  = newPeriod_ms / 1000;
    timerSpec.it_value.tv_nsec = (newPeriod_ms % 1000) * 1000000;

    if (mode == PERIODIC)
    {
        timerSpec.it_interval.tv_sec  = timerSpec.it_value.tv_sec;
        timerSpec.it_interval.tv_nsec = timerSpec.it_value.tv_nsec;
    }
    return start();
}

bool LinuxTimer::setPeriodFromISR(uint32_t newPeriod_ms)
{
    return setPeriod(newPeriod_ms);
}

[[nodiscard]] uint32_t LinuxTimer::getPeriod() const
{
    return timerSpec.it_value.tv_sec * 1000 + timerSpec.it_value.tv_nsec / 1000000;
}

[[nodiscard]] LinuxTimer::Mode LinuxTimer::getMode() const
{
    return mode;
}

[[nodiscard]] uint32_t LinuxTimer::getTimeout() const
{
    itimerspec currentSpec{};
    if (timer_gettime(timerId, &currentSpec) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to get timer information: %s", strerror(errno));
        exit(errno);
    }
    return currentSpec.it_value.tv_sec * 1000 + currentSpec.it_value.tv_nsec / 1000000;
}

[[nodiscard]] uint32_t LinuxTimer::getTimeoutTime() const
{
    itimerspec currentSpec{};
    if (timer_gettime(timerId, &currentSpec) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to get timer information: %s", strerror(errno));
        exit(errno);
    }
    timespec now{};
    clock_gettime(CLOCKID, &now);
    return (currentSpec.it_value.tv_sec + now.tv_sec) * 1000 + (currentSpec.it_value.tv_nsec + now.tv_nsec) / 1000000;
}
