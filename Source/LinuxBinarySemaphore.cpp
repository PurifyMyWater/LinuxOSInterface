#include "LinuxBinarySemaphore.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

LinuxBinarySemaphore::LinuxBinarySemaphore(bool& result)
{
    if (result = (sem_init(&semaphore, 0, 0) == 0); !result)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to initialize semaphore: %s", strerror(errno));
    }
}

LinuxBinarySemaphore::~LinuxBinarySemaphore()
{
    if (sem_destroy(&semaphore) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to destroy semaphore: %s", strerror(errno));
        exit(errno);
    }
}

void LinuxBinarySemaphore::signal()
{
    if (sem_post(&semaphore) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to signal semaphore: %s", strerror(errno));
        exit(errno);
    }
}

bool LinuxBinarySemaphore::wait(uint32_t max_time_to_wait_ms)
{
    const timespec ts  = msToTimespec(max_time_to_wait_ms);
    error_t        res = sem_timedwait(&semaphore, &ts);
    if (res == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to wait on semaphore: %s", strerror(errno));
    }
    return res == 0;
}
