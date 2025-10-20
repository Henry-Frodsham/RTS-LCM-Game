#pragma once
#include "InputDevice.h"
#include "EventBus.h"

// reads input states
class InputListener {
public:
	void update();

private:
	// event queue unnecessary since input will only control local UI and viewport
	EventBus* InputBus;

	InputDevice* Device;
};