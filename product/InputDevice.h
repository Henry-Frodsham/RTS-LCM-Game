// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>

#include "Player.h"

// stores which input device it is to control how the input should be read
enum InputDeviceType { NONE, KBM, CONTROLLER };

// the raw SDL_Joystick level indices for a specific physical pad, resolved once
// from that pad's SDL_GameController mapping (see
// InputListener::ResolveBindings).
//
// nothing in the codebase should ever assume "button 0 is A" or "axis 0 is the
// left stick X" - those indices differ per manufacturer, per platform and per
// driver. every index in here is either a real, checked value or -1 meaning
// "this pad does not report that control".
struct ControllerBindings {
  int LeftTriggerAxis = -1;
  int RightTriggerAxis = -1;

  int LeftStickXAxis = -1;
  int LeftStickYAxis = -1;
  int RightStickXAxis = -1;
  int RightStickYAxis = -1;

  int FaceButtonA = -1;
  int FaceButtonB = -1;
  int FaceButtonX = -1;
  int FaceButtonY = -1;

  int LeftShoulder = -1;
  int RightShoulder = -1;
  int Start = -1;

  // false means SDL had no controller mapping for this device, so every index
  // above is still -1 and callers must fall back to a guess (and say so)
  bool Resolved = false;
};

// binds a device to a specific player using a device
struct InputDevice {
  // SDL doesnt support GUID for KBM
  // however this isnt an issue since multiple KBM's arent supported
  SDL_Joystick* Controller;

  InputDeviceType InputType;

  // only meaningful when InputType == CONTROLLER
  ControllerBindings Bindings;

  InputDevice(SDL_Joystick* Joy, InputDeviceType Type)
      : Controller(Joy), InputType(Type) {}
};