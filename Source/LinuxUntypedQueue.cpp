#include "LinuxUntypedQueue.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

LinuxUntypedQueue::LinuxUntypedQueue(const uint32_t maxMessages, const uint32_t messageSize, bool& result) :
    maxMessages(maxMessages), messageSize(messageSize)
{
    // Create a unique queue name using process ID and timestamp
    snprintf(queueName, sizeof(queueName), "/osinterface_queue_%d_%d_%d", getpid(), osMillis(), rand());
    result = createQueue();
}

LinuxUntypedQueue::~LinuxUntypedQueue()
{
    deleteQueue();
}

uint32_t LinuxUntypedQueue::length()
{
    mq_attr attr{};
    if (mq_getattr(mqd, &attr) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to get queue attributes: %s", strerror(errno));
        return 0;
    }
    return attr.mq_curmsgs;
}

uint32_t LinuxUntypedQueue::size()
{
    return maxMessages;
}

uint32_t LinuxUntypedQueue::available()
{
    const uint32_t currentLength = this->length();
    return maxMessages - currentLength;
}

bool LinuxUntypedQueue::isEmpty()
{
    return this->length() == 0;
}

bool LinuxUntypedQueue::isFull()
{
    return this->length() >= maxMessages;
}

void LinuxUntypedQueue::reset()
{

    if (createQueue() != true)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to reset queue");
    }
}

bool LinuxUntypedQueue::createQueue()
{
    mq_attr attr{};
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = maxMessages;
    attr.mq_msgsize = messageSize;
    attr.mq_curmsgs = 0;

    mqd = mq_open(queueName, O_CREAT | O_RDWR | O_EXCL, 0644, &attr);
    if (mqd == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to create message queue %s: %s", queueName, strerror(errno));
        return false;
    }
    return true;
}

void LinuxUntypedQueue::deleteQueue()
{
    if (mqd != -1)
    {
        mq_close(mqd);
        mq_unlink(queueName);
        mqd = -1;
    }
}

bool LinuxUntypedQueue::receive(void* message, const uint32_t maxTimeToWait_ms)
{
    const timespec ts     = msToTimespec(maxTimeToWait_ms);
    const ssize_t  result = mq_timedreceive(mqd, static_cast<char*>(message), messageSize, nullptr, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to receive from queue: %s", strerror(errno));
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::receiveFromISR(void* message)
{
    const timespec ts     = msToTimespec(0);
    const ssize_t  result = mq_timedreceive(mqd, static_cast<char*>(message), messageSize, nullptr, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to receive from queue (ISR): %s", strerror(errno));
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToBack(const void* message, const uint32_t maxTimeToWait_ms)
{
    const timespec ts     = msToTimespec(maxTimeToWait_ms);
    const int      result = mq_timedsend(mqd, static_cast<const char*>(message), messageSize, 0, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to send to queue: %s", strerror(errno));
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToBackFromISR(const void* message)
{
    const timespec ts     = msToTimespec(0);
    const int      result = mq_timedsend(mqd, static_cast<const char*>(message), messageSize, 0, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to send to queue (ISR): %s", strerror(errno));
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToFront(const void* message, const uint32_t maxTimeToWait_ms)
{
    const timespec     ts      = msToTimespec(maxTimeToWait_ms);
    constexpr uint32_t maxPrio = 31;
    const int          result  = mq_timedsend(mqd, static_cast<const char*>(message), messageSize, maxPrio, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to send to front of queue: %s", strerror(errno));
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToFrontFromISR(const void* message)
{
    const timespec     ts      = msToTimespec(0);
    constexpr uint32_t maxPrio = 31;
    const int          result  = mq_timedsend(mqd, static_cast<const char*>(message), messageSize, maxPrio, &ts);

    if (result == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to send to front of queue (ISR): %s", strerror(errno));
        return false;
    }
    return true;
}

uint32_t LinuxUntypedQueue::osMillis()
{
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return timespecToMs(ts);
}
