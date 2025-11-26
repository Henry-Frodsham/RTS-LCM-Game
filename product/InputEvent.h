//Copyright © 2025 Henry Frodsham
#pragma once
#include <chrono>
#include <SDL2/SDL.h>

// only used for text prompts since actual game input is unified between KBM and controller
struct RawKBEvent {
	SDL_KeyboardEvent Key;
	bool KeyUp;

	RawKBEvent(SDL_KeyboardEvent KBKey, bool UpState)
		: Key(KBKey)
		, KeyUp(UpState){ }

};

// unified event for controller button input and key input
struct RawButtonEvent {
	SDL_JoyButtonEvent Button;
	bool ButtonUp;
	RawButtonEvent(SDL_JoyButtonEvent RawButton, bool UpState)
		: Button(RawButton) 
		, ButtonUp(UpState){ }
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