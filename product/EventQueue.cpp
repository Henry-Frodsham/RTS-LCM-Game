//Copyright © 2025 Henry Frodsham
#include "EventQueue.h"

EventQueue::EventQueue(EventBus* DefaultBus)
    : AssumedBus(DefaultBus){}

//process the entire queue and delete
void EventQueue::Dispatch(EventBus& Bus){
    while (!Queue.empty()) {
        Queue.front()(Bus);
        Queue.pop();
    }
}

//process the queue using the assumed bus
void EventQueue::Dispatch() {
    if (!AssumedBus) {
        return;
    }

    while (!Queue.empty()) {
        Queue.front()(*AssumedBus);
        Queue.pop();
    }
}

// clear the event queue
void EventQueue::Reset() {
    while (!Queue.empty()) {
        Queue.pop();
    }
}