#include "LinuxTimer.h"
#include "OSInterface_Log.h"

#include <cstdlib>
#include <cstring>

void LinuxTimer::callbackWrapper(union sigval sv)
{
    if (LinuxTimer* timer = static_cast<LinuxTimer*>(sv.sival_ptr);
        timer != nullptr && timer->callbacFunction != nullptr)
    {
        OSInterfaceLogInfo("LinuxOSInterface", "timer callback invoked");
        timer->callbacFunction(timer->callbackArg);
    }
    else
    {
        OSInterfaceLogWarning("LinuxOSInterface", "Timer callback invoked but %s%s",
                              timer == nullptr ? "EspTimer instance is null" : "callback function is null for timer ",
                              timer == nullptr ? "" : timer->name);
    }
}

LinuxTimer::LinuxTimer(uint32_t period, OSInterface_Timer::Mode mode, OSInterfaceProcess callback, void* callbackArg,
                       const char* timerName)
{
    this->name = strdup(timerName);

    this->callbacFunction = callback;
    this->callbackArg     = callbackArg;

    this->timerSpec.it_value.tv_sec  = period / 1000;
    this->timerSpec.it_value.tv_nsec = (period % 1000) * 1000000;

    if (mode == PERIODIC)
    {
        this->timerSpec.it_interval.tv_sec  = period / 1000;
        this->timerSpec.it_interval.tv_nsec = (period % 1000) * 1000000;
    }

    this->sev.sigev_notify          = SIGEV_THREAD;
    this->sev.sigev_notify_function = callbackWrapper;
    this->sev.sigev_value.sival_ptr = this;

    timer_create(CLOCKID, &this->sev, &this->timerId);
}

LinuxTimer::~LinuxTimer()
{
    free(name);
    timer_delete(timerId);
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
    timer_gettime(timerId, &currentSpec);
    return currentSpec.it_value.tv_sec != 0 || currentSpec.it_value.tv_nsec != 0;
}

bool LinuxTimer::setPeriod(uint32_t newPeriod_ms)
{
    timerSpec.it_value.tv_sec  = newPeriod_ms / 1000;
    timerSpec.it_value.tv_nsec = (newPeriod_ms % 1000) * 1000000;

    if (timerSpec.it_interval.tv_sec != 0 || timerSpec.it_interval.tv_nsec != 0)
    {
        timerSpec.it_interval.tv_sec  = newPeriod_ms / 1000;
        timerSpec.it_interval.tv_nsec = (newPeriod_ms % 1000) * 1000000;
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
    return (timerSpec.it_interval.tv_sec == 0 && timerSpec.it_interval.tv_nsec == 0) ? ONE_SHOT : PERIODIC;
}

[[nodiscard]] uint32_t LinuxTimer::getTimeout() const
{
    itimerspec currentSpec{};
    timer_gettime(timerId, &currentSpec);
    return currentSpec.it_value.tv_sec * 1000 + currentSpec.it_value.tv_nsec / 1000000;
}

[[nodiscard]] uint32_t LinuxTimer::getTimeoutTime() const
{
    itimerspec currentSpec{};
    timer_gettime(timerId, &currentSpec);
    timespec now{};
    clock_gettime(CLOCKID, &now);
    return (currentSpec.it_value.tv_sec + now.tv_sec) * 1000 + (currentSpec.it_value.tv_nsec + now.tv_nsec) / 1000000;
}
