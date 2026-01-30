#ifndef OSLINUXINTERFACE_H
#define OSLINUXINTERFACE_H

#include "OSInterface.h"

class LinuxOSInterface : public OSInterface
{
public:
    void                         osSleep(uint32_t ms) override;
    uint32_t                     osMillis() override;
    OSInterface_Mutex*           osCreateMutex() override;
    OSInterface_BinarySemaphore* osCreateBinarySemaphore() override;

    OSInterface_Timer* osCreateTimer(uint32_t period, OSInterface_Timer::Mode mode, OSInterfaceProcess callback,
                                     void* callbackArg, const char* timerName) override;

    OSInterface_UntypedQueue* osCreateUntypedQueue(uint32_t maxMessages, uint32_t messageSize) override;

    void* osMalloc(uint32_t size) override;
    void  osFree(void* ptr) override;
    void  osRunProcess(OSInterfaceProcess process, void* arg) override;
    void  osRunProcess(OSInterfaceProcess process, const char* processName, void* arg) override;
};

#endif // OSLINUXINTERFACE_H
