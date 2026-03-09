#include "Utils.h"
#include "Config.h"

timespec msToTimespec(uint32_t ms)
{
    timespec ts{};
    clock_gettime(CLOCKID, &ts);
    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000;

    // Handle overflow of nanoseconds
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    return ts;
}

uint32_t timespecToMs(const timespec& ts)
{
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
