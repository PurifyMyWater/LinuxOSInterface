#include "LinuxMutex.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

LinuxMutex::LinuxMutex(bool& result)
{
    pthread_mutexattr_t attr;
    if (result = (pthread_mutexattr_init(&attr) == 0); !result)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to initialize mutex attr: %s", strerror(errno));
        return;
    }
    if (result = (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) == 0); !result)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to initialize mutex: %s", strerror(errno));
        return;
    }
    pthread_mutex_init(&mutex, &attr);
}

LinuxMutex::~LinuxMutex()
{
    if (pthread_mutex_unlock(&mutex) != 0 && pthread_mutex_destroy(&mutex) != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to destroy mutex: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void LinuxMutex::signal()
{
    if (const error_t res = pthread_mutex_unlock(&mutex); res != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to unlock mutex: %s", strerror(res));
        exit(EXIT_FAILURE);
    }
}

bool LinuxMutex::wait(uint32_t max_time_to_wait_ms)
{
    const timespec ts  = msToTimespec(max_time_to_wait_ms);
    error_t        res = pthread_mutex_timedlock(&mutex, &ts);
    if (res != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to lock mutex: %s", strerror(res));
    }
    return res == 0;
}
