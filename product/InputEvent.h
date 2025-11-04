//Copyright © 2025 Henry Frodsham
#pragma once
#include <chrono>
#include <SDL2/SDL.h>

// only used for text prompts since actual game input is unified between KBM and controller
struct RawKBEvent {
	SDL_KeyboardEvent Key;

	RawKBEvent(SDL_KeyboardEvent KBKey)
		: Key(KBKey) { }

};

// unified event for controller button input and key input
struct RawButtonEvent {
	SDL_ControllerButtonEvent Button;

	RawButtonEvent(SDL_ControllerButtonEvent RawButton)
		: Button(RawButton) { }
};

// mouse movement event
struct RawCursorEvent {
	SDL_MouseMotionEvent Cursor;
	RawCursorEvent(SDL_MouseMotionEvent MCursor)
		: Cursor(MCursor) { }
};

// joystick event
struct RawAxisEvent {
	SDL_JoyAxisEvent Axis;
	RawAxisEvent(SDL_JoyAxisEvent JAxis)
		: Axis(JAxis) { }
};