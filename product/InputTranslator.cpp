//Copyright © 2025 Henry Frodsham
#include "InputTranslator.h"

InputTranslator::InputTranslator() {
	InputEvents = EventBus();
	WaitingEvents = EventQueue(&InputEvents);

	InputEvents.Subscribe<RawKBEvent>(std::bind(&InputTranslator::TranslateRawKB, this, std::placeholders::_1));
	InputEvents.Subscribe<RawButtonEvent>(std::bind(&InputTranslator::TranslateRawButton, this, std::placeholders::_1));
	InputEvents.Subscribe<RawCursorEvent>(std::bind(&InputTranslator::TranslateRawCursor, this, std::placeholders::_1));
	InputEvents.Subscribe<RawAxisEvent>(std::bind(&InputTranslator::TranslateRawAxis, this, std::placeholders::_1));
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
		if (C >= 'a' && C <= 'z' && (modifiers & KMOD_SHIFT)) {
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

}
void InputTranslator::TranslateRawAxis(RawAxisEvent Event){

}
