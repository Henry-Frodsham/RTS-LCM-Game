//Copyright © 2025 Henry Frodsham
#pragma once
#include "InputDevice.h"
#include "EventBus.h"

// reads input states, each device has a unique listener and is queued onto an event bus to stop SDL events being read at the same time
class InputListener {
public:
	void update();

private:
	// event queue unnecessary since input will only control local UI and viewport
	EventBus* InputBus;

	InputDevice* Device;
};