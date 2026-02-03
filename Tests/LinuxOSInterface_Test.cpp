#include "LinuxOSInterface.h"
#include "gtest/gtest.h"

static LinuxOSInterface linuxOSInterface;

TEST(LinuxOSInterface, timeTest)
{
    uint32_t timeToSleep = 10;
    uint32_t repeat      = 10;
    bool     flag        = false;

    for (uint32_t i = 0; i < repeat; i++)
    {
        uint32_t millis = linuxOSInterface.osMillis();
        linuxOSInterface.osSleep(timeToSleep);
        uint32_t millis2 = linuxOSInterface.osMillis();

        EXPECT_GE(millis2, millis + timeToSleep);
        if (millis2 >= millis + timeToSleep + 100)
        {
            flag = true;
        }
    }

    if (flag)
    {
        FAIL() << "Sleep slept for too long";
    }
}

TEST(LinuxOSInterface, osMallocSimpleAlloc)
{
    void* ptr = linuxOSInterface.osMalloc(100);
    ASSERT_NE(ptr, nullptr);
    linuxOSInterface.osFree(ptr);
}

TEST(LinuxOSInterface, osMallocZeroAlloc)
{
    void* ptr = linuxOSInterface.osMalloc(0);
    ASSERT_EQ(ptr, nullptr);
    linuxOSInterface.osFree(ptr);
}

TEST(LinuxOSInterface, osMallocLargeAlloc)
{
    void* ptr = linuxOSInterface.osMalloc(1024 * 1024 * 6);
    ASSERT_NE(ptr, nullptr);
    linuxOSInterface.osFree(ptr);
}

TEST(LinuxOSInterface, osMallocMultipleAlloc)
{
    void* ptr1 = linuxOSInterface.osMalloc(100);
    void* ptr2 = linuxOSInterface.osMalloc(100);
    void* ptr3 = linuxOSInterface.osMalloc(100);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    linuxOSInterface.osFree(ptr1);
    linuxOSInterface.osFree(ptr2);
    linuxOSInterface.osFree(ptr3);
}

void runProcessTest(void* arg)
{
    volatile bool* processRun = static_cast<bool*>(arg);
    linuxOSInterface.osSleep(100);
    *processRun = true;
}

TEST(LinuxOSInterface, runProcessTest)
{
    volatile bool processRun = false;
    void* arg = (void*) (&processRun);

    auto millis = linuxOSInterface.osMillis();
    linuxOSInterface.osRunProcess(runProcessTest, arg);

    while (!processRun && (millis + 1000 > linuxOSInterface.osMillis()))
    {
        linuxOSInterface.osSleep(10);
    }
    EXPECT_TRUE(processRun);
}

TEST(LinuxOSInterface, osCreateMutexNotNull)
{
    EXPECT_NE(linuxOSInterface.osCreateMutex(), nullptr);
}

TEST(LinuxOSInterface, osCreateBinarySemaphoreNotNull)
{
    EXPECT_NE(linuxOSInterface.osCreateBinarySemaphore(), nullptr);
}
