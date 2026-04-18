#include "LinuxMutex.h"
#include "gtest/gtest.h"

TEST(LinuxMutex, waitTest)
{
    bool result;
    OSInterface_Mutex* mutex = new LinuxMutex(result);
    EXPECT_TRUE(result);
    EXPECT_TRUE(mutex != nullptr);
    EXPECT_TRUE(mutex->wait(10));
    delete mutex;
}

TEST(LinuxMutex, normalTest)
{
    bool result;
    OSInterface_Mutex* mutex = new LinuxMutex(result);
    EXPECT_TRUE(result);
    ASSERT_NE(mutex, nullptr);

    // Lock the mutex
    ASSERT_TRUE(mutex->wait(10000));
    // Flag to check if the second thread was able to lock the mutex
    volatile bool secondThreadLocked = false;

    // Create a second thread that tries to lock the mutex
    std::thread t(
        [&]
        {
            EXPECT_TRUE(mutex->wait(10000));
            secondThreadLocked = true;
            mutex->signal();
        });

    // Sleep for a short time to ensure the second thread attempts to lock the mutex
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // The second thread should not have been able to lock the mutex yet
    EXPECT_FALSE(secondThreadLocked);

    // Unlock the mutex
    mutex->signal();

    // Wait for the second thread to finish
    t.join();

    // Now the second thread should have been able to lock the mutex
    EXPECT_TRUE(secondThreadLocked);
    delete mutex;
}

TEST(LinuxMutex, timeoutTest)
{
    bool result;
    OSInterface_Mutex* mutex = new LinuxMutex(result);
    EXPECT_TRUE(result);
    ASSERT_NE(mutex, nullptr);

    // Lock the mutex
    ASSERT_TRUE(mutex->wait(10000));
    // Flag to check if the second thread was able to lock the mutex
    volatile bool secondThreadLocked = false;

    // Create a second thread that tries to lock the mutex
    std::thread t(
        [&]
        {
            auto res = mutex->wait(50);
            EXPECT_FALSE(res);
            if (res)
            {
                secondThreadLocked = true;
                mutex->signal();
            }
        });

    // Sleep for a short time to ensure the second thread attempts to lock the mutex
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // The second thread should not have been able to lock the mutex yet
    EXPECT_FALSE(secondThreadLocked);

    // Unlock the mutex
    mutex->signal();

    // Wait for the second thread to finish
    t.join();

    EXPECT_FALSE(secondThreadLocked);
    delete mutex;
}
