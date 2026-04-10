// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>

#include "Player.h"

// stores which input device it is to control how the input should be read
enum InputDeviceType { NONE, KBM, CONTROLLER };

// binds a device to a specific player using a device
struct InputDevice {
  // SDL doesnt support GUID for KBM
  // however this isnt an issue since multiple KBM's arent supported
  SDL_Joystick* Controller;

  InputDeviceType InputType;
  Sint8 LeftTriggerRawAxis = -1;
  Sint8 RightTriggerRawAxis = -1;
  InputDevice(SDL_Joystick* Joy, InputDeviceType Type)
      : Controller(Joy), InputType(Type) {}
};
