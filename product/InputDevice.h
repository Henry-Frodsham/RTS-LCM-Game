#pragma once
#include "Player.h"

enum InputDeviceType {
	NONE,
	KBM,
	CONTROLLER
};

struct InputDevice {
	Player* Player;
	InputDeviceType InputType;

	InputDevice()
		: Player(nullptr)
		, InputType(InputDeviceType::NONE) { }
};