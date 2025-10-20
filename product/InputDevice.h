//Copyright © 2025 Henry Frodsham
#pragma once
#include "Player.h"

// stores which input device it is to control how the input should be read
enum InputDeviceType {
	NONE,
	KBM,
	CONTROLLER
};

// binds a device to a specific player using a device
struct InputDevice {
	Player* Player;
	InputDeviceType InputType;

	InputDevice()
		: Player(nullptr)
		, InputType(InputDeviceType::NONE) { }
};