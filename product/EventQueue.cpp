//Copyright © 2025 Henry Frodsham
#include "EventQueue.h"

EventQueue::EventQueue(EventBus* DefaultBus)
    : AssumedBus(DefaultBus){}

//process the entire queue and delete
void EventQueue::Dispatch(EventBus& Bus) {
    std::queue<std::function<void(EventBus&)>> QueueCopy;

    {
        std::lock_guard<std::mutex> Lock(QueueMutex);
        QueueCopy = std::move(Queue);  
    }  

    while (!QueueCopy.empty()) {
        QueueCopy.front()(Bus);
        QueueCopy.pop();
    }
}

//process the queue using the assumed bus
void EventQueue::Dispatch() {
    std::queue<std::function<void(EventBus&)>> QueueCopy;

    // transfer ownership of the queue to QueueCopy whilst processing events
    // allows Enqueue to be safely used at the same time as Dispatch is processing events
    {
        std::lock_guard<std::mutex> Lock(QueueMutex);
        if (!AssumedBus) {
            return;
        }
        QueueCopy = std::move(Queue);
    }

    while (!QueueCopy.empty()) {
        QueueCopy.front()(*AssumedBus);
        QueueCopy.pop();
    }
}

// clear the event queue
void EventQueue::Reset() {
    std::lock_guard<std::mutex> Lock(QueueMutex);
    while (!Queue.empty()) {
        Queue.pop();
    }
}