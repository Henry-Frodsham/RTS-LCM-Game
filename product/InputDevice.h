//Copyright © 2025 Henry Frodsham
#pragma once
#include "Player.h"
#include <SDL2/SDL.h>

// stores which input device it is to control how the input should be read
enum InputDeviceType {
	NONE,
	KBM,
	CONTROLLER
};

// binds a device to a specific player using a device
struct InputDevice {
	//SDL doesnt support GUID for KBM
	//however this isnt an issue since multiple KBM's arent supported
	SDL_JoystickGUID ControllerPersistentId;

	InputDeviceType InputType;

	InputDevice(SDL_JoystickGUID GUID, InputDeviceType Type)
		: ControllerPersistentId(GUID)
		, InputType(Type) { }
};