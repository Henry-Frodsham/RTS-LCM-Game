//Copyright © 2025 Henry Frodsham
#pragma once
#include <queue>
#include <memory>
#include <functional>
#include "EventBus.h"

class EventQueue {
public:
    EventQueue() {}
    ~EventQueue() {}

    //template function implementation needs to be here
    //add an event to the queue
    template<typename EventType>
    void Enqueue(EventType&& Event) {
        Queue.push([Event = std::forward<EventType>(Event)](EventBus& Bus) mutable {
            Bus.Publish(std::move(Event));
            });
    }

    void Dispatch(EventBus& bus);

    void Reset();

private:
    std::queue<std::function<void(EventBus&)>> Queue;
};