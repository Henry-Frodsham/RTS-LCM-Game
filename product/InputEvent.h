#pragma once
#include <chrono>

// only used for text prompts since actual game input is unified between KBM and controller
struct RawKBEvent {
	std::chrono::system_clock::time_point TimeStamp;
};

// unified event for controller button input and key input
struct ButtonEvent {
	std::chrono::system_clock::time_point TimeStamp;
};

struct CursorEvent {
	float Pitch,Yaw;
	std::chrono::system_clock::time_point TimeStamp;
	CursorEvent() 
		: TimeStamp()
		, Pitch(0.f)
		, Yaw(0.f) { }
};