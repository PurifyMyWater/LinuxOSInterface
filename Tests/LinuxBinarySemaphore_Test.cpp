#include "LinuxBinarySemaphore.h"
#include "gtest/gtest.h"

TEST(LinuxBinarySemaphore, initTest)
{
    bool result;
    OSInterface_BinarySemaphore* semaphore = new LinuxBinarySemaphore(result);
    ASSERT_TRUE(result);
    EXPECT_TRUE(semaphore != nullptr);
    EXPECT_FALSE(semaphore->wait(10));
    delete semaphore;
}

TEST(LinuxBinarySemaphore, waitSignalTest)
{
    bool result;
    OSInterface_BinarySemaphore* semaphore = new LinuxBinarySemaphore(result);
    ASSERT_TRUE(result);
    EXPECT_TRUE(semaphore != nullptr);
    semaphore->signal();
    EXPECT_TRUE(semaphore->wait(10));
    delete semaphore;
}

TEST(LinuxBinarySemaphore, waitSignalWaitTest)
{
    bool result;
    OSInterface_BinarySemaphore* semaphore = new LinuxBinarySemaphore(result);
    ASSERT_TRUE(result);
    EXPECT_TRUE(semaphore != nullptr);
    semaphore->signal();
    EXPECT_TRUE(semaphore->wait(10));
    EXPECT_FALSE(semaphore->wait(10));
    delete semaphore;
}

TEST(LinuxBinarySemaphore, normalTest)
{
    bool result;
    OSInterface_BinarySemaphore* semaphore = new LinuxBinarySemaphore(result);
    ASSERT_TRUE(result);
    ASSERT_NE(semaphore, nullptr);
    semaphore->signal();

    // Lock the semaphore
    ASSERT_TRUE(semaphore->wait(10000));
    // Flag to check if the second thread was able to lock the semaphore.
    volatile bool secondThreadLocked = false;

    // Create a second thread that tries to lock the semaphore
    std::thread t(
        [&]
        {
            EXPECT_TRUE(semaphore->wait(10000));
            secondThreadLocked = true;
            semaphore->signal();
        });

    // Sleep for a short time to ensure the second thread attempts to lock the semaphore
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // The second thread should not have been able to lock the semaphore yet
    EXPECT_FALSE(secondThreadLocked);

    // Unlock the semaphore
    semaphore->signal();

    // Wait for the second thread to finish
    t.join();

    // Now the second thread should have been able to lock the semaphore
    EXPECT_TRUE(secondThreadLocked);
    delete semaphore;
}

TEST(LinuxBinarySemaphore, timeoutTest)
{
    bool result;
    OSInterface_BinarySemaphore* semaphore = new LinuxBinarySemaphore(result);
    ASSERT_TRUE(result);
    ASSERT_NE(semaphore, nullptr);
    semaphore->signal();

    // Lock the semaphore
    ASSERT_TRUE(semaphore->wait(10000));
    // Flag to check if the second thread was able to lock the semaphore
    volatile bool secondThreadLocked = false;

    // Create a second thread that tries to lock the semaphore
    std::thread t(
        [&]
        {
            auto res = semaphore->wait(50);
            EXPECT_FALSE(res);
            if (res)
            {
                secondThreadLocked = true;
                semaphore->signal();
            }
        });

    // Sleep for a short time to ensure the second thread attempts to lock the semaphore
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // The second thread should not have been able to lock the semaphore yet
    EXPECT_FALSE(secondThreadLocked);

    // Unlock the semaphore
    semaphore->signal();

    // Wait for the second thread to finish
    t.join();

    EXPECT_FALSE(secondThreadLocked);
    delete semaphore;
}
