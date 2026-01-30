#ifndef LINUXMUTEX_H
#define LINUXMUTEX_H

#include "OSInterface_Mutex.h"

#include <mutex>

class LinuxMutex final : public OSInterface_Mutex
{
public:
    LinuxMutex();

    ~LinuxMutex() override;

    void signal() override;

    bool wait(uint32_t max_time_to_wait_ms) override;

private:
    pthread_mutex_t mutex{};
};

#endif // LINUXMUTEX_H
