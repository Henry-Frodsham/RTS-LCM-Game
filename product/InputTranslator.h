// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <unordered_set>

#include "ConfigManager.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GameAction.h"
#include "InputDevice.h"
#include "InputEvent.h"
#include "OverlayEvent.h"
#include "ResizeEvent.h"
// listens to its registered device and converts to actual game actions
class InputTranslator {
 public:
  InputTranslator(InputDevice* Device, float VPWidth, float VPHeight);
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

  void ResizeViewPortDimensions(ResizedViewPortEvent Event);

 private:
  EventBus* InputEvents;

  // active actions, remains until button/key released
  std::unordered_set<GameAction> ActiveActions;

  // actual key states, case sensitive and only used for text prompts
  std::unordered_set<char> KeyStates;

  std::unordered_set<Uint8> ButtonStates;

  float CursorSensitivity;
  float JoystickDeadzone;
  float ViewPortWidth;
  float ViewPortHeight;

  // std::unordered_set<> ButtonStates;

  std::vector<float> CursorPos;

  Ogre::Vector2f RelativeMotion;

  std::vector<float> JoyStickStates;

  void TranslateRawKB(RawKBEvent Event);
  void TranslateRawButton(RawButtonEvent Event);
  void TranslateRawCursor(RawCursorEvent Event);
  void TranslateRawAxis(RawAxisEvent Event);

  float ApplyDeadzone(float Value, float Deadzone);
};
