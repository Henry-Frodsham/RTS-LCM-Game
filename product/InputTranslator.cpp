//Copyright © 2025 Henry Frodsham
#include "InputTranslator.h"
#include "InputAnalyser.h"

InputTranslator::InputTranslator(InputDevice* Device){
	ManagedDevice = Device;
	InputEvents = new EventBus();
	WaitingEvents = new EventQueue(InputEvents);

	CursorPos = { 0.f,0.f };
	JoyStickStates = { 0.f,0.f };
	InputEvents->Subscribe<RawKBEvent>(std::bind(&InputTranslator::TranslateRawKB, this, std::placeholders::_1));
	InputEvents->Subscribe<RawButtonEvent>(std::bind(&InputTranslator::TranslateRawButton, this, std::placeholders::_1));
	InputEvents->Subscribe<RawCursorEvent>(std::bind(&InputTranslator::TranslateRawCursor, this, std::placeholders::_1));
	InputEvents->Subscribe<RawAxisEvent>(std::bind(&InputTranslator::TranslateRawAxis, this, std::placeholders::_1));

	CursorSensitivity = 100.f;
	JoystickDeadzone = 0.1f;

	Ogre::RenderWindowDescription WindowInfo = RenderSystem::GetInstance().GetPrimaryWindowInformation();
	ScreenWidth = float(WindowInfo.width);
	ScreenHeight = float(WindowInfo.height);

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
	Uint8 ButtonIndex = Event.Button.button;

	if (ButtonIndex == 0) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::USE);
		}
		else {
			ActiveActions.insert(GameAction::USE);
		}
	}
	else if (ButtonIndex == 1) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::BACK);
		}
		else {
			ActiveActions.insert(GameAction::BACK);
		}
	}
	else if (ButtonIndex == 2) {
		if (Event.ButtonUp) {
			ActiveActions.erase(GameAction::CONTXT);
		}
		else {
			ActiveActions.insert(GameAction::CONTXT);
		}
	}

	if (Event.ButtonUp) {
		ButtonStates.erase(Event.Button.button);
	}
	else {
		ButtonStates.insert(Event.Button.button);
	}
}
void InputTranslator::TranslateRawCursor(RawCursorEvent Event){
	const SDL_MouseMotionEvent& Motion = Event.Cursor;
	std::vector<float> CursorVec{
		static_cast<float>(Motion.x),
		static_cast<float>(Motion.y)
	};
	RelativeMotion = Ogre::Vector2f(
		CursorVec[0] - CursorPos[0],
		CursorVec[1] - CursorPos[1]
	);
	CursorPos = CursorVec;
}
void InputTranslator::TranslateRawAxis(RawAxisEvent Event){
	const SDL_JoyAxisEvent& SDL_Ev = Event.Axis;

	float NormalizedValue = static_cast<float>(SDL_Ev.value) / 32767.0f;

	NormalizedValue = std::clamp(NormalizedValue, -1.0f, 1.0f);

	// Left stick X axis
	if (SDL_Ev.axis == 0) {
		JoyStickStates[0] = NormalizedValue;
	}
	// Left stick Y axis
	else if (SDL_Ev.axis == 1) {
		JoyStickStates[1] = NormalizedValue;
	}

	// Right stick X axis
	else if (SDL_Ev.axis == 2) {
		// TODO: Implement right stick functionality
	}
	// Right stick Y axis
	else if (SDL_Ev.axis == 3) {
		// TODO: Implement right stick functionality
	}
}

int InputTranslator::GetNumPressedKeys() {
	if (ManagedDevice->InputType == InputDeviceType::KBM) {
		return KeyStates.size();
	}
	else {
		return ButtonStates.size();
	}
}

std::vector<float> InputTranslator::GetCurrentAxis() {
	return CursorPos;
}

//converts a joystick float to one that takes into account the deadzone
float InputTranslator::ApplyDeadzone(float Value, float Deadzone) {
	// ignore movements smaller than the deadzone
	if (std::abs(Value) < Deadzone) {
		return 0.0f;
	}

	// velocity scaling starts at the end of the deadzone
	// stops a sudden accelleration when leaving deadzone
	float sign = (Value > 0.0f) ? 1.0f : -1.0f;
	return sign * ((std::abs(Value) - Deadzone) / (1.0f - Deadzone));
}

void InputTranslator::Update(float DeltaTime) {
	//reset relative motion before events are processed, so if theres a tick without motion then its cleared
	RelativeMotion = Ogre::Vector2f(0.f,0.f);
	WaitingEvents->Dispatch();
	if (ManagedDevice->InputType == InputDeviceType::CONTROLLER) {
		float stickX = ApplyDeadzone(JoyStickStates[0], JoystickDeadzone);
		float stickY = ApplyDeadzone(JoyStickStates[1], JoystickDeadzone);

		float velocityX = stickX * CursorSensitivity * DeltaTime;
		float velocityY = stickY * CursorSensitivity * DeltaTime;

		//update and clamp both axis
		CursorPos[0] += velocityX;
		CursorPos[1] += velocityY;

		RelativeMotion = Ogre::Vector2f(
			velocityX,
			velocityY
		);

		CursorPos[0] = std::clamp(CursorPos[0], 0.f, ScreenWidth);
		CursorPos[1] = std::clamp(CursorPos[1], 0.f, ScreenHeight);
	}
}

Ogre::Vector2f InputTranslator::GetRelativeMotion() {
	return RelativeMotion;
}
bool InputTranslator::HasRelativeMotion() {
	if (RelativeMotion != Ogre::Vector2f(0.f, 0.f)) {
		return true;
	}
	return false;
}