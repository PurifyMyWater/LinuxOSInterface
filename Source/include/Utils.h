#ifndef LINUXUTILS_H
#define LINUXUTILS_H

#include <cstdint>
#include <ctime>

timespec msToTimespec(uint32_t ms);
uint32_t timespecToMs(const timespec& ts);

#endif // LINUXUTILS_H
