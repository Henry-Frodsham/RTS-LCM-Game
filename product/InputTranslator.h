//Copyright © 2025 Henry Frodsham
#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include "GameAction.h"
#include "InputEvent.h"
#include <unordered_set>

// listens to its registered device and converts to actual game actions
class InputTranslator {
public:
	InputTranslator();
	//~InputTranslator();

	bool HasAction(GameAction Action);
	bool getKeyState(char Key);

	EventQueue WaitingEvents;

private:
	EventBus InputEvents;
	
	// active actions, remains until button/key released
	std::unordered_set<GameAction> ActiveActions;

	// actual key states, case sensitive and only used for text prompts
	std::unordered_set<char> KeyStates;

	void TranslateRawKB(RawKBEvent Event);
	void TranslateRawButton(RawButtonEvent Event);
	void TranslateRawCursor(RawCursorEvent Event);
	void TranslateRawAxis(RawAxisEvent Event);
};