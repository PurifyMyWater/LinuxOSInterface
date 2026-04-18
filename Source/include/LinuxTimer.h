#ifndef LINUXTIMER_H
#define LINUXTIMER_H

#include "OSInterface.h"
#include "OSInterface_Timer.h"

#include <csignal>
#include <ctime>

class LinuxTimer final : public OSInterface_Timer
{
public:
    LinuxTimer(uint32_t period, OSInterface_Timer::Mode mode, OSInterfaceProcess callback, void* callbackArg,
               const char* timerName, bool& result);

    ~LinuxTimer() override;

    bool start() override;

    bool startFromISR() override;

    bool stop() override;

    bool stopFromISR() override;

    [[nodiscard]] bool isRunning() const override;

    bool setPeriod(uint32_t newPeriod_ms) override;

    bool setPeriodFromISR(uint32_t newPeriod_ms) override;

    [[nodiscard]] uint32_t getPeriod() const override;

    [[nodiscard]] Mode getMode() const override;

    [[nodiscard]] uint32_t getTimeout() const override;

    [[nodiscard]] uint32_t getTimeoutTime() const override;

private:
    char* name;

    timer_t timerId{};

    Mode mode;

    OSInterfaceProcess callbackFunction;
    void*              callbackArg;

    sigevent sev{};

    itimerspec timerSpec{};

    static void initializeSignalSystem();

    static void signalHandler(int sig, siginfo_t* si, void* uc);
};

#endif // LINUXTIMER_H
