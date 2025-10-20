//Copyright © 2025 Henry Frodsham
#pragma once
#include <queue>
#include <any>
#include <iostream>
#include "EventBus.h"

//queues events using FIFO before passing off to event bus
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