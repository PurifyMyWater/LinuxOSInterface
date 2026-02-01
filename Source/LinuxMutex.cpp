#include "LinuxMutex.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

LinuxMutex::LinuxMutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex, &attr);
}

LinuxMutex::~LinuxMutex()
{
    pthread_mutex_destroy(&mutex);
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
