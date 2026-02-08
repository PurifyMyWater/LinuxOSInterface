#include "LinuxTimer.h"
#include "LinuxOSInterface.h"
#include "OSInterface_Timer.h"
#include "gtest/gtest.h"
#include <cstdint>
#include <cmath>

static LinuxOSInterface linuxOSInterface;

void timerCallback(void* arg)
{
    int* counter = static_cast<int*>(arg);
    if (counter != nullptr)
    {
        (*counter)++;
    }
}

TEST(LinuxTimer, startOneShot)
{
    int                counter = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, (void*)(&counter), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(counter, 1);
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(counter, 1);
    delete timer;
}

TEST(LinuxTimer, startPeriodic)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::PERIODIC, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(1, called);
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(2, called);
    delete timer;
}

TEST(LinuxTimer, stopOneshot)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    ASSERT_TRUE(timer->stop());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(0, called);
    delete timer;
}

TEST(LinuxTimer, stopPeriodic)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::PERIODIC, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(1, called);
    ASSERT_TRUE(timer->stop());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(1, called);
    delete timer;
}

TEST(LinuxTimer, isRunning)
{
    OSInterface_Timer* timer = new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, nullptr, "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_FALSE(timer->isRunning());
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(10);
    ASSERT_TRUE(timer->isRunning());
    linuxOSInterface.osSleep(110);
    ASSERT_FALSE(timer->isRunning());
    linuxOSInterface.osSleep(110);
    delete timer;
}

TEST(LinuxTimer, setPeriod)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(1, called);
    ASSERT_TRUE(timer->setPeriod(200));
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(1, called);
    linuxOSInterface.osSleep(110);
    ASSERT_EQ(2, called);
    delete timer;
}

TEST(LinuxTimer, getPeriod)
{
    OSInterface_Timer* timer = new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, nullptr, "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_EQ(100, timer->getPeriod());
    ASSERT_TRUE(timer->setPeriod(200));
    linuxOSInterface.osSleep(10);
    ASSERT_EQ(200, timer->getPeriod());
    delete timer;
}

TEST(LinuxTimer, getMode)
{
    OSInterface_Timer* timer;

    timer = new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, nullptr, "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_EQ(OSInterface_Timer::ONE_SHOT, timer->getMode());
    delete timer;

    timer = new LinuxTimer(100, OSInterface_Timer::PERIODIC, timerCallback, nullptr, "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_EQ(OSInterface_Timer::PERIODIC, timer->getMode());
    delete timer;
}

TEST(LinuxTimer, getTimeout)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(20);
    uint32_t timeout = timer->getTimeout();
    linuxOSInterface.osSleep(timeout - 10);
    ASSERT_EQ(0, called);
    linuxOSInterface.osSleep(20);
    ASSERT_EQ(1, called);
    delete timer;
}

TEST(LinuxTimer, getTimeoutTime)
{
    int                called = 0;
    OSInterface_Timer* timer =
        new LinuxTimer(100, OSInterface_Timer::ONE_SHOT, timerCallback, (void*)(&called), "test_timer");

    ASSERT_NE(timer, nullptr);
    ASSERT_TRUE(timer->start());
    linuxOSInterface.osSleep(20);
    uint32_t timeoutTime = timer->getTimeoutTime();

    ASSERT_LE(std::abs(static_cast<int64_t>(timeoutTime) - static_cast<int64_t>(linuxOSInterface.osMillis() + 80)), 10);
    while (called == 0 && linuxOSInterface.osMillis() < timeoutTime + 10)
    {
        linuxOSInterface.osSleep(10);
    }
    ASSERT_EQ(1, called);
    delete timer;
}
