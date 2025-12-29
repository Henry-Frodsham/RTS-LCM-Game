// Copyright © 2025 Henry Frodsham
#include <doctest/doctest.h>

#include "InputListener.h"
#include "InputTranslator.h"
#include "RenderSystem.h"

TEST_CASE("InputListener - constructor") { CHECK_NOTHROW(InputListener Obj()); }

TEST_CASE("InputListener - succesful translation") {
	RenderSystem& RS = RenderSystem::GetInstance();
	InputListener Obj = InputListener(RS.GetSDLWindow());
	InputDevice* MockDevice = new InputDevice(nullptr, InputDeviceType::KBM);

	InputTranslator IT = InputTranslator(MockDevice,0.f,0.f);

	Obj.AddListenerQueue(MockDevice, IT.WaitingEvents);

	//InputListener forwards events directly from sdl so just mock that behaviour

	//mimic an extremely bare event
	SDL_KeyboardEvent MockEvent = SDL_KeyboardEvent{ 0,0,0,0,0,0,0,SDL_Keysym{SDL_Scancode::SDL_SCANCODE_B,SDL_KeyCode::SDLK_b} };

	IT.WaitingEvents->Enqueue(RawKBEvent(MockEvent, false));

	IT.Update(0.f);

	CHECK(IT.GetNumPressedKeys() != 0);
}

TEST_CASE("InputListener - succesful erasure of key press when the key is released") {
	RenderSystem& RS = RenderSystem::GetInstance();
	InputListener Obj = InputListener(RS.GetSDLWindow());
	InputDevice* MockDevice = new InputDevice(nullptr, InputDeviceType::KBM);

	InputTranslator IT = InputTranslator(MockDevice, 0.f,0.f);

	Obj.AddListenerQueue(MockDevice, IT.WaitingEvents);

	//InputListener forwards events directly from sdl so just mock that behaviour

	//mimic an extremely bare event
	SDL_KeyboardEvent MockEvent = SDL_KeyboardEvent{ 0,0,0,0,0,0,0,SDL_Keysym{SDL_Scancode::SDL_SCANCODE_B,SDL_KeyCode::SDLK_b} };

	IT.WaitingEvents->Enqueue(RawKBEvent(MockEvent, false));

	IT.Update(0.f);

	CHECK(IT.GetNumPressedKeys() != 0);

	IT.WaitingEvents->Enqueue(RawKBEvent(MockEvent, true));

	IT.Update(0.f);

	CHECK(IT.GetNumPressedKeys() == 0);
}
