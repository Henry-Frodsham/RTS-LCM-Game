// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

#include "EventBus.h"

class EventQueue {
 private:
  std::queue<std::function<void(EventBus&)>> Queue;

  mutable std::mutex QueueMutex;

  EventBus* AssumedBus;

 public:
  explicit EventQueue(EventBus* DefaultBus = nullptr);
  ~EventQueue() {}

  // template function implementation needs to be here
  // add an event to the queue
  template <typename EventType>
  void Enqueue(EventType&& Event) {
    std::lock_guard<std::mutex> Lock(QueueMutex);
    Queue.push([Event = std::forward<EventType>(Event)](EventBus& Bus) mutable {
      Bus.Publish(std::move(Event));
    });
  }

  void Dispatch(EventBus& bus);
  void Dispatch();

  void Reset();
};
