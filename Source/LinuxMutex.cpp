#include "LinuxMutex.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

LinuxMutex::LinuxMutex(bool& result)
{
    pthread_mutexattr_t attr;
    error_t             res;
    result = false;
    if (res = pthread_mutexattr_init(&attr); res != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to initialize mutex attr: %s", strerror(res));
        return;
    }
    if (res = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK); res != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to initialize mutex: %s", strerror(res));
        return;
    }
    pthread_mutex_init(&mutex, &attr);
    result = true;
}

LinuxMutex::~LinuxMutex()
{
    pthread_mutex_trylock(&mutex);  // blocking mutex if unlocked as unlocking an unlocked mutex is undefined behavior
    signal();                       // make sure to unblock mutex before destroying it
    if (const error_t res = pthread_mutex_destroy(&mutex); res != 0)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to destroy mutex: %s", strerror(res));
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
