// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ActionCommand.h"
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
  InputTranslator(InputDevice* Device, float VPWidth, float VPHeight,
                  int ThreadNum, std::vector<float> SDim);

  bool HasAction(GameAction Action);

  ActionContext GetActionContext(GameAction Action);

  bool getKeyState(char Key);

  EventQueue* WaitingEvents;

  InputDevice* ManagedDevice;
  std::vector<float> ScreenDimensions;
  int GetNumPressedKeys();

  std::vector<float> GetCurrentAxis();
  Ogre::Vector2f GetRelativeMotion();

  bool HasRelativeMotion();
  bool HoldingRMBorLT();
  void Update(float DeltaTime);

  void ResizeViewPortDimensions(ResizedViewPortEvent Event);

  std::vector<float> GetViewPortDimensions();
  std::vector<float> GetScreenDimensions();
  // might seem like an odd choice, but the queue to publish here is private
  // so this bus is soley for classes interested in listening to game actions
  EventBus* ActionBus;

 private:
  EventBus* InputEvents;

  EventQueue* ActionQueue;
  // active actions, remains until button/key released
  std::unordered_map<GameAction, ActionContext> ActiveActions;

  // actual key states, case sensitive and only used for text prompts
  std::unordered_set<char> KeyStates;

  std::unordered_set<Uint8> ButtonStates;

  float CursorSensitivity;
  float JoystickDeadzone;
  float ViewPortWidth;
  float ViewPortHeight;

  int ThreadNumber;
  // std::unordered_set<> ButtonStates;

  std::vector<float> CursorPos;

  Ogre::Vector2f RelativeMotion;

  std::vector<float> JoyStickStates;

  void TranslateRawKB(RawKBEvent Event);
  void TranslateRawButton(RawButtonEvent Event);
  void TranslateRawCursor(RawCursorEvent Event);
  void TranslateRawAxis(RawAxisEvent Event);
  void TranslateRawMouseButton(RawMouseButtonEvent Event);
  void TranslateRawTriggerEvent(RawTriggerEvent Event);
  void HandleReconnectPrompt(ReconnectControllerPromptEvent Event);
  void HideReconnectPrompt(ReconnectControllerSuccessEvent Event);
  float ApplyDeadzone(float Value, float Deadzone);

  std::vector<bool> MouseButtonStates = std::vector<bool>{false, false, false};
  std::vector<bool> TriggerStates = std::vector<bool>{false, false};
  ErrorReporter* TranslationErrorReporter;
};
