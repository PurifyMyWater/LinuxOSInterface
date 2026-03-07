#include "LinuxOSInterface.h"
#include "LinuxBinarySemaphore.h"
#include "LinuxMutex.h"
#include "LinuxUntypedQueue.h"

#include <thread>

#define CONFIG_USE_BUSY_SLEEP 0

uint32_t LinuxOSInterface::osMillis()
{
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#if CONFIG_USE_BUSY_SLEEP
void linuxSleep(uint32_t ms)
{
    uint32_t start = linuxMillis();
    while (linuxMillis() - start < ms)
        ;
}
#else
void LinuxOSInterface::osSleep(const uint32_t ms)
{
    timespec ts{};
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, nullptr);
}
#endif

OSInterface_Mutex* LinuxOSInterface::osCreateMutex()
{
    return new LinuxMutex();
}

OSInterface_BinarySemaphore* LinuxOSInterface::osCreateBinarySemaphore()
{
    return new LinuxBinarySemaphore();
}

OSInterface_Timer* LinuxOSInterface::osCreateTimer(uint32_t period, OSInterface_Timer::Mode mode,
                                                   OSInterfaceProcess callback, void* callbackArg,
                                                   const char* timerName)
{
    return nullptr;
}

OSInterface_UntypedQueue* LinuxOSInterface::osCreateUntypedQueue(uint32_t maxMessages, uint32_t messageSize)
{
    bool               result;
    LinuxUntypedQueue* queue = new LinuxUntypedQueue(maxMessages, messageSize, result);
    if (!result)
    {
        OSInterfaceLogError("OSInterface", "Failed to create untyped queue");
        delete queue;
        return nullptr;
    }
    return queue;
}

void* LinuxOSInterface::osMalloc(const uint32_t size)
{
    return size == 0 ? nullptr : malloc(size);
}

void LinuxOSInterface::osFree(void* ptr)
{
    free(ptr);
}

void LinuxOSInterface::osRunProcess(OSInterfaceProcess process, void* arg)
{
    osRunProcess(process, "NewProcess", arg);
}

void LinuxOSInterface::osRunProcess(OSInterfaceProcess process, const char* processName, void* arg)
{
    OSInterfaceLogInfo("OSInterface", "Running process %s", processName);
    std::thread t(process, arg);
    t.detach();
}
