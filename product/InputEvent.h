// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>

#include <chrono>

// raw SDL keyboard events from the poll event loop (see InputListener)
// the event is then translated by an input translator
// example usage:
// SomeQueue.enqueue(RawKBEvent(Event, false))
struct RawKBEvent {
  SDL_KeyboardEvent Key;
  bool KeyUp;

  RawKBEvent(SDL_KeyboardEvent KBKey, bool UpState)
      : Key(KBKey), KeyUp(UpState) {}
};

// raw SDL event for button presses on a controller also from the poll event
// loop then translated by an input translator example usage:
// SomeQueue.enqueue(RawButtonEvent(Event, false))
struct RawButtonEvent {
  SDL_JoyButtonEvent Button;
  bool ButtonUp;
  RawButtonEvent(SDL_JoyButtonEvent RawButton, bool UpState)
      : Button(RawButton), ButtonUp(UpState) {}
};

// raw mouse movement event generated from the poll event loop
// the contained data is a coordinated based on the relative position of the
// mouse on the screen example usage SomeQueue.enqueue(RawCursorEvent(Event))
struct RawCursorEvent {
  SDL_MouseMotionEvent Cursor;
  explicit RawCursorEvent(SDL_MouseMotionEvent MCursor) : Cursor(MCursor) {}
};

// raw individual joystick movement event generated from the poll event loop
// the contained SDL event stores the pitch and yaw of the joystick
// example usage
// SomeQueue.enqueue(RawAxisEvent(Event))
struct RawAxisEvent {
  SDL_JoyAxisEvent Axis;
  explicit RawAxisEvent(SDL_JoyAxisEvent JAxis) : Axis(JAxis) {}
};

// SDL event for mouse buttons e.g mouse4
struct RawMouseButtonEvent {
  SDL_MouseButtonEvent Button;
  bool Released;
  RawMouseButtonEvent(SDL_MouseButtonEvent RawButton, bool IsReleased)
      : Button(RawButton), Released(IsReleased) {}
};
// SDL event for triggers e.g LT and RT
struct RawTriggerEvent {
  SDL_JoyAxisEvent AxisEvent;
  float NormalizedValue;
  RawTriggerEvent(SDL_JoyAxisEvent AE, float NV)
      : AxisEvent(AE), NormalizedValue(NV) {}
};

struct RawMouseWheelEvent {
  SDL_MouseWheelEvent WheelEvent;
  RawMouseWheelEvent(SDL_MouseWheelEvent WheelEv) : WheelEvent(WheelEv) {}
};

struct ReconnectControllerPromptEvent {
  int PositionInQueue;
  ReconnectControllerPromptEvent(int PosInQ) : PositionInQueue(PosInQ) {}
};

struct ReconnectControllerSuccessEvent {
  ReconnectControllerSuccessEvent() {}
};
