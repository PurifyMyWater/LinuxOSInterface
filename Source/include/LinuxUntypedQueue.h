#ifndef LINUXUNTYPEDQUEUE_H
#define LINUXUNTYPEDQUEUE_H

#include "OSInterface_UntypedQueue.h"

#include <mqueue.h>
#include <atomic>

class LinuxUntypedQueue final : public OSInterface_UntypedQueue
{
public:
    LinuxUntypedQueue(uint32_t maxMessages, uint32_t messageSize, bool& result);

    ~LinuxUntypedQueue() override;

    uint32_t length() override;

    uint32_t size() override;

    uint32_t available() override;

    bool isEmpty() override;

    bool isFull() override;

    void reset() override;

    bool receive(void* message, uint32_t maxTimeToWait_ms) override;

    bool receiveFromISR(void* message) override;

    bool sendToBack(const void* message, uint32_t maxTimeToWait_ms) override;

    bool sendToBackFromISR(const void* message) override;

    bool sendToFront(const void* message, uint32_t maxTimeToWait_ms) override;

    bool sendToFrontFromISR(const void* message) override;

private:
    mqd_t                  mqd{};
    char                   queueName[256]{};
    uint32_t               maxMessages;
    uint32_t               messageSize;
    std::atomic<uint32_t>  currentPriority;

    bool            createQueue();
    void            deleteQueue();
    static uint32_t osMillis();
};
#endif // LINUXUNTYPEDQUEUE_H
