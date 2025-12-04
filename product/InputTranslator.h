//Copyright © 2025 Henry Frodsham
#pragma once
#include <unordered_set>
#include "EventBus.h"
#include "EventQueue.h"
#include "GameAction.h"
#include "InputEvent.h"
#include "InputDevice.h"
#include "OverlayEvent.h"
#include "ConfigManager.h"
#include <OGRE/Ogre.h>

// listens to its registered device and converts to actual game actions
class InputTranslator {
public:
	InputTranslator(InputDevice* Device);
	//~InputTranslator();

	bool HasAction(GameAction Action);
	bool getKeyState(char Key);

	EventQueue* WaitingEvents;

	InputDevice* ManagedDevice;

	int GetNumPressedKeys();

	std::vector<float> GetCurrentAxis();
	Ogre::Vector2f GetRelativeMotion();

	bool HasRelativeMotion();
	void Update(float DeltaTime);

private:
	EventBus* InputEvents;
	
	// active actions, remains until button/key released
	std::unordered_set<GameAction> ActiveActions;

	// actual key states, case sensitive and only used for text prompts
	std::unordered_set<char> KeyStates;

	std::unordered_set<Uint8> ButtonStates;

	float CursorSensitivity;              
	float JoystickDeadzone;               
	float ScreenWidth;                    
	float ScreenHeight;

	//std::unordered_set<> ButtonStates;

	std::vector<float> CursorPos;

	Ogre::Vector2f RelativeMotion;

	std::vector<float> JoyStickStates;


	void TranslateRawKB(RawKBEvent Event);
	void TranslateRawButton(RawButtonEvent Event);
	void TranslateRawCursor(RawCursorEvent Event);
	void TranslateRawAxis(RawAxisEvent Event);

	float ApplyDeadzone(float Value, float Deadzone);
};