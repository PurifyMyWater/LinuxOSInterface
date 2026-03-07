#include "LinuxOSInterface.h"
#include "gtest/gtest.h"

static LinuxOSInterface linuxOSInterface;

TEST(LinuxOSInterface, queueSendToBack)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int msg1 = 1;
    int msg2 = 2;
    int msg3 = 3;
    EXPECT_TRUE(queue->sendToBack(&msg1, 100));
    EXPECT_TRUE(queue->sendToBack(&msg2, 100));
    EXPECT_TRUE(queue->sendToBack(&msg3, 100));
    EXPECT_EQ(queue->length(), 3);

    int received = 0;
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 1);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 2);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 3);

    delete queue;
}

TEST(LinuxOSInterface, queueSendToBackFromISR)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int message = 123;
    EXPECT_TRUE(queue->sendToBackFromISR(&message));
    EXPECT_EQ(queue->length(), 1);

    delete queue;
}

TEST(LinuxOSInterface, queueSendToFront)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int msg1 = 1;
    int msg2 = 2;
    int msg3 = 3;
    EXPECT_TRUE(queue->sendToFront(&msg1, 100)); // Priority 1
    EXPECT_TRUE(queue->sendToFront(&msg2, 100)); // Priority 2
    EXPECT_TRUE(queue->sendToFront(&msg3, 100)); // Priority 3
    EXPECT_EQ(queue->length(), 3);

    // Higher priority messages come first (3, 2, 1)
    int received = 0;
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 3);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 2);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 1);

    delete queue;
}

TEST(LinuxOSInterface, queueSendToFrontFromISR)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int message = 77;
    EXPECT_TRUE(queue->sendToFrontFromISR(&message));
    EXPECT_EQ(queue->length(), 1);

    delete queue;
}

TEST(LinuxOSInterface, queueMixedSendOperations)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int msg1 = 1;
    int msg2 = 2;
    int msg3 = 3;
    int msg4 = 4;

    EXPECT_TRUE(queue->sendToBack(&msg1, 100));  // Priority 0: [1]
    EXPECT_TRUE(queue->sendToBack(&msg2, 100));  // Priority 0: [1, 2]
    EXPECT_TRUE(queue->sendToFront(&msg3, 100)); // Priority 1
    EXPECT_TRUE(queue->sendToFront(&msg4, 100)); // Priority 2

    EXPECT_EQ(queue->length(), 4);

    // Highest priority first: msg4 (prio 2), msg3 (prio 1)
    // Then low priority messages: msg1, msg2 (both prio 0, FIFO order)
    int received = 0;
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 4);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 3);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 1);
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, 2);

    delete queue;
}

TEST(LinuxOSInterface, queueReceive)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int sendMsg = 42;
    EXPECT_TRUE(queue->sendToBack(&sendMsg, 100));

    int recvMsg = 0;
    EXPECT_TRUE(queue->receive(&recvMsg, 100));
    EXPECT_EQ(recvMsg, 42);
    EXPECT_EQ(queue->length(), 0);

    delete queue;
}

TEST(LinuxOSInterface, queueReceiveFromISR)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int sendMsg = 123;
    EXPECT_TRUE(queue->sendToBackFromISR(&sendMsg));

    int recvMsg = 0;
    EXPECT_TRUE(queue->receiveFromISR(&recvMsg));
    EXPECT_EQ(recvMsg, 123);
    EXPECT_EQ(queue->length(), 0);

    delete queue;
}

TEST(LinuxOSInterface, queueSize)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    EXPECT_EQ(queue->size(), 10);

    delete queue;
}

TEST(LinuxOSInterface, queueAvailable)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    EXPECT_EQ(queue->available(), 10);

    int message = 42;
    queue->sendToBack(&message, 100);
    EXPECT_EQ(queue->available(), 9);

    delete queue;
}

TEST(LinuxOSInterface, queueIsEmpty)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    EXPECT_TRUE(queue->isEmpty());

    int message = 42;
    queue->sendToBack(&message, 100);
    EXPECT_FALSE(queue->isEmpty());

    delete queue;
}

TEST(LinuxOSInterface, queueIsFull)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(2, sizeof(int));
    ASSERT_NE(queue, nullptr);

    EXPECT_FALSE(queue->isFull());

    int message1 = 1;
    int message2 = 2;
    queue->sendToBack(&message1, 100);
    EXPECT_FALSE(queue->isFull());

    queue->sendToBack(&message2, 100);
    EXPECT_TRUE(queue->isFull());

    delete queue;
}

TEST(LinuxOSInterface, queueReset)
{
    OSInterface_UntypedQueue* queue = linuxOSInterface.osCreateUntypedQueue(10, sizeof(int));
    ASSERT_NE(queue, nullptr);

    int message = 42;
    queue->sendToBack(&message, 100);
    queue->sendToBack(&message, 100);
    EXPECT_EQ(queue->length(), 2);

    queue->reset();
    EXPECT_EQ(queue->length(), 0);
    EXPECT_TRUE(queue->isEmpty());

    // Verify the queue still functions correctly after reset.
    int newMessage = 99;
    EXPECT_TRUE(queue->sendToBack(&newMessage, 100));
    EXPECT_EQ(queue->length(), 1);

    int received = 0;
    EXPECT_TRUE(queue->receive(&received, 100));
    EXPECT_EQ(received, newMessage);
    EXPECT_TRUE(queue->isEmpty());
    delete queue;
}
