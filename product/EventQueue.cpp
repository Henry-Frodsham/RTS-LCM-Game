//Copyright © 2025 Henry Frodsham
#include "EventQueue.h"


//process the entire queue and delete
void EventQueue::Dispatch(EventBus& Bus){
    while (!Queue.empty()) {
        Queue.front()(Bus);
        Queue.pop();
    }
}

// clear the event queue
void EventQueue::Reset() {
    while (!Queue.empty()) {
        Queue.pop();
    }
}