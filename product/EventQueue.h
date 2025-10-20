//Copyright © 2025 Henry Frodsham
#pragma once
#include <queue>
#include <any>
#include <iostream>
#include "EventBus.h"

// queues events using FIFO before passing off to event bus, prevents race conditions
// example usage for queuing events:
// NewEventQueue.enqueue(std::make_unique<std::any>(SomeEvent()))
// example usage for processing events onto an event bus:
// NewEventQueue.dispatch(&SomeEventBus)
class EventQueue {
public:
	EventQueue() {}
	~EventQueue() {};

	void enqueue(std::unique_ptr<std::any> Event);
	
	void dispatch(EventBus& Bus);

	void reset();

private:
	std::queue<std::unique_ptr<std::any>> Queue;
	
};