//Copyright © 2025 Henry Frodsham
#include "InputTranslator.h"
#include "InputAnalyser.h"

InputTranslator::InputTranslator(InputDevice* Device){
	ManagedDevice = Device;
	InputEvents = EventBus();
	WaitingEvents = EventQueue(&InputEvents);

	CursorPos = { 0.f,0.f };
	JoyStickStates = { 0.f,0.f };
	InputEvents.Subscribe<RawKBEvent>(std::bind(&InputTranslator::TranslateRawKB, this, std::placeholders::_1));
	InputEvents.Subscribe<RawButtonEvent>(std::bind(&InputTranslator::TranslateRawButton, this, std::placeholders::_1));
	InputEvents.Subscribe<RawCursorEvent>(std::bind(&InputTranslator::TranslateRawCursor, this, std::placeholders::_1));
	InputEvents.Subscribe<RawAxisEvent>(std::bind(&InputTranslator::TranslateRawAxis, this, std::placeholders::_1));

#ifdef _DEBUG
	InputAnalyser::GetInstance().RegisterNew(this);
#endif

}

bool InputTranslator::HasAction(GameAction Action) {
	return ActiveActions.contains(Action);
}

bool InputTranslator::getKeyState(char Key) {
	return KeyStates.contains(Key);
}

void InputTranslator::TranslateRawKB(RawKBEvent Event){
	SDL_Keycode Key = Event.Key.keysym.sym;

	if (Key == SDLK_e) {
		if (Event.KeyUp) {
			ActiveActions.erase(GameAction::USE);
		}
		else {
			ActiveActions.insert(GameAction::USE);
		}
	}
	else if (Key == SDLK_q) {
		if (Event.KeyUp) {
			ActiveActions.erase(GameAction::BACK);
		}
		else {
			ActiveActions.insert(GameAction::BACK);
		}
	}
	else if (Key == SDLK_r) {
		if (Event.KeyUp) {
			ActiveActions.erase(GameAction::CONTXT);
		}
		else {
			ActiveActions.insert(GameAction::CONTXT);
		}
	}

	//sdl2 not case sensitive by default so check the state of shift and capslock
	Uint16 modifiers = Event.Key.keysym.mod;

	if (Key >= 32 && Key <= 126) {
		char C = (char)Key;

		// check state of shift and capslock
		// if pressed then convert to upper case
		if (C >= 'a' && C <= 'z' && ((modifiers & KMOD_SHIFT) || (modifiers & KMOD_CAPS))) {
			C = C - 32;
		}

		//only store the pressed keys so remove if keys been released
		if (Event.KeyUp) {
			KeyStates.erase(C);
		}
		else {
			KeyStates.insert(C);
		}
	}

}
void InputTranslator::TranslateRawButton(RawButtonEvent Event){
	SDL_GameControllerButton Button = (SDL_GameControllerButton)Event.Button.button;

	if (Button == SDL_CONTROLLER_BUTTON_A) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::USE);
		}
		else {
			ActiveActions.insert(GameAction::USE);
		}
	}
	else if (Button == SDL_CONTROLLER_BUTTON_B) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::BACK);
		}
		else {
			ActiveActions.insert(GameAction::BACK);
		}
	}
	else if (Button == SDL_CONTROLLER_BUTTON_X) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::CONTXT);
		}
		else {
			ActiveActions.insert(GameAction::CONTXT);
		}
	}

}
void InputTranslator::TranslateRawCursor(RawCursorEvent Event){
	const SDL_MouseMotionEvent& Motion = Event.Cursor;
	std::vector<float> CursorVec{
		static_cast<float>(Motion.x),
		static_cast<float>(Motion.y)
	};

	CursorPos = CursorVec;
}
void InputTranslator::TranslateRawAxis(RawAxisEvent Event){
	const SDL_JoyAxisEvent& SDL_Ev = Event.Axis;
	// normalise to -1.f - 1.f
	float NormalisedValue = (float(SDL_Ev.value) / 32767.5f);

	// the left stick acts as a mouse for precise actions
	// left stick x axis
	if (SDL_Ev.axis == 0) {
		JoyStickStates[0] += NormalisedValue;
	}
	// left stick y axis
	else if (SDL_Ev.axis == 1) {
		JoyStickStates[1] += NormalisedValue;
	}

	// ignore these for now, the right stick is for snap events instead of using the joystick as a cursor
	// right stick x axis
	if (SDL_Ev.axis == 0) {

	}
	// right stick y axis
	else if (SDL_Ev.axis == 1) {

	}
}

int InputTranslator::GetNumPressedKeys() {
	return KeyStates.size();
}

std::vector<float> InputTranslator::GetCurrentAxis() {
	return CursorPos;
}

void InputTranslator::Update() {
	WaitingEvents.Dispatch();


	//todo : normalize to delta time
	CursorPos[0] += JoyStickStates[0];
	CursorPos[1] += JoyStickStates[1];

	//todo, get the actual screen size
	CursorPos[0] = abs(std::min(800.f, CursorPos[0]));
	CursorPos[1] = abs(std::min(600.f, CursorPos[1]));
}