#ifndef LINUXBINARYSEMAPHORE_H
#define LINUXBINARYSEMAPHORE_H

#include "OSInterface_BinarySemaphore.h"

#include <semaphore.h>

class LinuxBinarySemaphore final : public OSInterface_BinarySemaphore
{
public:
    LinuxBinarySemaphore(bool& result);

    ~LinuxBinarySemaphore() override;

    void signal() override;

    bool wait(uint32_t max_time_to_wait_ms) override;

private:
    sem_t semaphore{};
};
#endif // LINUXBINARYSEMAPHORE_H
