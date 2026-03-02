#include "LinuxUntypedQueue.h"
#include "OSInterface_Log.h"
#include "Utils.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

LinuxUntypedQueue::LinuxUntypedQueue(uint32_t maxMessages, uint32_t messageSize) :
    maxMessages_(maxMessages), messageSize_(messageSize)
{
    // Create a unique queue name using process ID and timestamp
    char queueName[256];
    snprintf(queueName, sizeof(queueName), "/osinterface_queue_%d_%d", getpid(), osMillis());
    queueName_ = queueName;

    mq_attr attr{
      .mq_flags = 0,
      .mq_maxmsg  = maxMessages,
      .mq_msgsize = messageSize,
      .mq_curmsgs = 0
    };
    

    mqd_ = mq_open(queueName_.c_str(), O_CREAT | O_RDWR | O_EXCL, 0644, &attr);
    if (mqd_ == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to create message queue %s: %s", queueName_.c_str(),
                            strerror(errno));
        exit(errno);
    }
}

LinuxUntypedQueue::~LinuxUntypedQueue()
{
    if (mqd_ != -1)
    {
        mq_close(mqd_);
        mq_unlink(queueName_.c_str());
    }
}

uint32_t LinuxUntypedQueue::length()
{
    mq_attr attr{};
    if (mq_getattr(mqd_, &attr) == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to get queue attributes: %s", strerror(errno));
        return 0;
    }
    return attr.mq_curmsgs;
}

uint32_t LinuxUntypedQueue::size()
{
    return maxMessages_;
}

uint32_t LinuxUntypedQueue::available()
{
    const uint32_t currentLength = length();
    return maxMessages_ - currentLength;
}

bool LinuxUntypedQueue::isEmpty()
{
    return length() == 0;
}

bool LinuxUntypedQueue::isFull()
{
    return length() >= maxMessages_;
}

void LinuxUntypedQueue::reset()
{
    mq_close(mqd_);
    mq_unlink(queueName_.c_str());

    mq_attr attr{};
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = maxMessages_;
    attr.mq_msgsize = messageSize_;
    attr.mq_curmsgs = 0;

    mqd_ = mq_open(queueName_.c_str(), O_CREAT | O_RDWR | O_EXCL, 0644, &attr);
    if (mqd_ == -1)
    {
        OSInterfaceLogError("LinuxOSInterface", "Failed to recreate message queue %s: %s", queueName_.c_str(),
                            strerror(errno));
        exit(errno);
    }
}

bool LinuxUntypedQueue::receive(void* message, uint32_t maxTimeToWait_ms)
{
    const timespec ts     = msToTimespec(maxTimeToWait_ms);
    const ssize_t  result = mq_timedreceive(mqd_, static_cast<char*>(message), messageSize_, nullptr, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to receive from queue: %s", strerror(errno));
        }
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::receiveFromISR(void* message)
{
    const timespec ts     = msToTimespec(0);
    const ssize_t  result = mq_timedreceive(mqd_, static_cast<char*>(message), messageSize_, nullptr, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to receive from queue (ISR): %s", strerror(errno));
        }
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToBack(const void* message, uint32_t maxTimeToWait_ms)
{
    const timespec ts     = msToTimespec(maxTimeToWait_ms);
    const int      result = mq_timedsend(mqd_, static_cast<const char*>(message), messageSize_, 0, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to send to queue: %s", strerror(errno));
        }
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToBackFromISR(const void* message)
{
    timespec  ts{0, 0};
    const int result = mq_timedsend(mqd_, static_cast<const char*>(message), messageSize_, 0, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to send to queue (ISR): %s", strerror(errno));
        }
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToFront(const void* message, uint32_t maxTimeToWait_ms)
{
    const timespec     ts      = msToTimespec(maxTimeToWait_ms);
    constexpr uint32_t maxPrio = 31;
    const int          result  = mq_timedsend(mqd_, static_cast<const char*>(message), messageSize_, maxPrio, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to send to front of queue: %s", strerror(errno));
        }
        return false;
    }
    return true;
}

bool LinuxUntypedQueue::sendToFrontFromISR(const void* message)
{
    timespec           ts{0, 0};
    constexpr uint32_t maxPrio = 31;
    const int          result  = mq_timedsend(mqd_, static_cast<const char*>(message), messageSize_, maxPrio, &ts);

    if (result == -1)
    {
        if (errno != ETIMEDOUT)
        {
            OSInterfaceLogError("LinuxOSInterface", "Failed to send to front of queue (ISR): %s", strerror(errno));
        }
        return false;
    }
    return true;
}

uint32_t LinuxUntypedQueue::osMillis()
{
    timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
