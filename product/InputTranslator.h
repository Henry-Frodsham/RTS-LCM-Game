// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>
#include <SDL2/SDL.h>

#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ActionCommand.h"
#include "ConfigEvent.h"
#include "ConfigManager.h"
#include "ErrorReporter.h"
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
  ~InputTranslator();

  InputTranslator(const InputTranslator&) = delete;
  InputTranslator& operator=(const InputTranslator&) = delete;

  bool HasAction(GameAction Action);

  ActionContext GetActionContext(GameAction Action);

  bool getKeyState(char Key);

  // state of a single mouse button, indexed by the SDL_BUTTON_* constant
  // itself (SDL_BUTTON_LEFT == 1, MIDDLE == 2, RIGHT == 3) so theres no
  // remapping step to get wrong
  bool IsMouseButtonDown(Uint8 SdlButton) const;

  // the "hold to orbit the camera" modifier, right mouse or left trigger
  bool HoldingRMBorLT() const;

  // the "add to the current selection rather than replacing it" modifier.
  // shift on a keyboard (either one, bound by name through
  // KeyBindAddToSelection) and the Y face button on a pad - Y being the one
  // face button no GameAction claims, so nothing had to be rebound to make
  // room for it
  bool HoldingAdditiveSelect() const;

  // current cursor position in normalised 0-1 viewport space, same
  // convention as ActionContext::MouseX/Y (see MakeContext) - lets a caller
  // build a StartRayTraceEvent for the current frame without waiting on a
  // click event
  std::vector<float> GetNormalizedCursorPosition() const {
    const float X = (ViewPortWidth > 0.f) ? CursorPos[0] / ViewPortWidth : 0.f;
    const float Y = (ViewPortHeight > 0.f) ? CursorPos[1] / ViewPortHeight : 0.f;
    return {X, Y};
  }

  int GetNumPressedKeys();

  std::vector<float> GetCurrentAxis();
  Ogre::Vector2f GetRelativeMotion();

  bool HasRelativeMotion();

  // ---- camera zoom ----
  // ZOOM CONTROL: the mouse scroll wheel and the LB/RB pad shoulder buttons
  // both drive the exact same value below. scrolling reports real notches
  // the instant they arrive; holding LB/RB is converted into notches at
  // ShoulderZoomNotchesPerSecond (see LoadConfiguration) so a pad feels the
  // same as spinning the wheel. everything downstream (ChangeOrbitDepthEvent,
  // ViewPortController::MoveCameraDepth, etc) only ever reads this pair of
  // getters and has no idea a second input source exists.
  //   LB (left shoulder)  == scroll up   == zoom in
  //   RB (right shoulder) == scroll down == zoom out
  // rounded whole notches, kept for the existing ChangeOrbitDepthEvent
  int GetMouseWheelY();
  // full float precision, high resolution wheels and trackpads report
  // fractions of a notch (SDL 2.0.18+)
  float GetPreciseMouseWheelY() const;

  void Update(float DeltaTime);

  void ResizeViewPortDimensions(ResizedViewPortEvent Event);

  // this instance's settings file has been rewritten - drop every cached copy
  // of it and read it again. published to this translator's own bus by its
  // GameInstance, so it runs on the instance thread that owns these values
  void ReloadConfiguration(ConfigAppliedEvent Event);

  std::vector<float> GetViewPortDimensions();
  std::vector<float> GetScreenDimensions();

  // which split screen instance this translator belongs to. everything on the
  // instance side names its overlays after it, so anything building one needs
  // to be able to ask rather than being handed the number separately
  int GetThreadNumber() const { return ThreadNumber; }

  EventQueue* WaitingEvents;

  InputDevice* ManagedDevice;
  std::vector<float> ScreenDimensions;

  // might seem like an odd choice, but the queue to publish here is private
  // so this bus is soley for classes interested in listening to game actions
  EventBus* ActionBus;

 private:
  // ---- raw event translation ----
  void TranslateRawKB(RawKBEvent Event);
  void TranslateRawButton(RawButtonEvent Event);
  void TranslateRawCursor(RawCursorEvent Event);
  void TranslateRawAxis(RawAxisEvent Event);
  void TranslateRawMouseButton(RawMouseButtonEvent Event);
  void TranslateRawTriggerEvent(RawTriggerEvent Event);
  void TranslateRawMouseWheelEvent(RawMouseWheelEvent Event);
  void HandleReconnectPrompt(ReconnectControllerPromptEvent Event);
  void HideReconnectPrompt(ReconnectControllerSuccessEvent Event);

  // ---- shared action plumbing ----
  // single place that turns "a control went down or up" into latched action
  // state plus the matching command. every input path goes through here so a
  // keyboard bind and a pad bind can never drift apart
  void ApplyDigitalAction(GameAction Action, bool Pressed);
  void PublishActionCommand(GameAction Action, const ActionContext& Context);
  void PublishPressCommand(bool Released);

  // ---- select gesture ----
  // left mouse and the right trigger are the same verb, so both drive the one
  // gesture below rather than each growing their own click handling. a press
  // opens it, cursor movement while its open extends it, a release closes it.
  // see DragSelectCommand for why a click and a box drag are one gesture
  void BeginSelectDrag();
  void UpdateSelectDrag();
  // Cancelled closes the gesture without picking anything, for when the
  // control that opened it goes away instead of being let go of
  void EndSelectDrag(bool Cancelled = false);
  void PublishSelectDragCommand(DragPhase Phase);
  // every consumer of ActionContext works in normalised 0-1 viewport space
  ActionContext MakeContext(bool JustPressed) const;

  // ---- setup ----
  void LoadConfiguration();
  void BuildKeyBindings();
  void RebuildPadButtonBindings();
  void CreateReconnectPrompt();
  void RefreshReconnectPrompt();
  std::string PromptOverlayName() const;
  std::string PromptBorderName() const;
  std::string PromptTextName() const;

  float ApplyDeadzone(float Value, float Deadzone) const;

  // raw "is this shoulder button currently down" check, used to fold LB/RB
  // into the scroll wheel's zoom pipeline. true == LB, false == RB
  bool IsShoulderHeld(bool LeftShoulder) const;

  EventBus* InputEvents;
  EventQueue* ActionQueue;
  ErrorReporter* TranslationErrorReporter;
  ConfigManager* InputConfig;

  // active actions, remains until button/key released
  std::unordered_map<GameAction, ActionContext> ActiveActions;

  // actual key states, case sensitive and only used for text prompts
  std::unordered_set<char> KeyStates;

  std::unordered_set<Uint8> ButtonStates;

  // bindings, all data driven rather than hardcoded if chains
  std::unordered_map<SDL_Keycode, GameAction> KeyBindings;
  // raw joystick button index -> action, rebuilt whenever the pads bindings
  // are resolved (including after a reconnect)
  std::unordered_map<int, GameAction> PadButtonBindings;

  // indexed by SDL_BUTTON_*, slot 0 is unused
  static constexpr std::size_t MouseButtonCount = SDL_BUTTON_X2 + 1;
  std::array<bool, MouseButtonCount> MouseButtonStates{};

  static constexpr std::size_t LeftTriggerSlot = 0;
  static constexpr std::size_t RightTriggerSlot = 1;
  std::array<bool, 2> TriggerStates{};

  // tuning, all read from config/default/InputSettings.json
  float CursorSensitivity = 1500.f;

  float ControllerOrbitSensitivity = 1.2f;

  float JoystickDeadzone = 0.1f;
  float TriggerThreshold = 0.1f;
  float RelativeMotionScale = 100.f;
  float RightDragThresholdPixels = 4.f;
  // how far the cursor has to travel from where the select verb went down
  // before the gesture counts as a box rather than a click. deliberately
  // looser than RightDragThresholdPixels - a click that picks a unit is worth
  // being forgiving about, an orbit that opens the wheel is not
  float SelectDragThresholdPixels = 6.f;


  bool RightClickOpensContextWheel = true;

  // how many "scroll wheel notches" a second of held LB or RB is worth. see
  // the ZOOM CONTROL comment above GetMouseWheelY
  float ShoulderZoomNotchesPerSecond = 6.f;

  std::vector<float> PromptPosition{0.2f, 0.3f};
  std::vector<float> PromptDimensions{0.24f, 0.03f};
  std::string PromptBorderMaterial = "RED";
  std::string PromptTextMaterial = "WHITE";

  float ViewPortWidth;
  float ViewPortHeight;

  int ThreadNumber;

  std::vector<float> CursorPos{0.f, 0.f};
  Ogre::Vector2f RelativeMotion{0.f, 0.f};

  std::vector<float> JoyStickStates{0.f, 0.f};

  // right mouse doubles as the camera orbit modifier, so a right *click* only
  // counts as a context action if the cursor didnt travel while it was held
  std::vector<float> RightDragOrigin{0.f, 0.f};
  bool RightButtonDragged = false;

  // the open select gesture, if there is one. origin is kept in pixels so the
  // threshold is a real screen distance rather than a fraction of whatever
  // this viewport happens to be sized at, and normalised only when published
  std::vector<float> SelectDragOrigin{0.f, 0.f};
  bool SelectDragActive = false;
  bool SelectDragExceeded = false;
  bool SelectDragAdditive = false;

  // shift, tracked as its own latch rather than read off a keysym modifier
  // mask - the mask only arrives with a key event, and the modifier has to be
  // readable at the moment a mouse button goes down
  bool AdditiveSelectKeyHeld = false;
  // the two keycodes KeyBindAddToSelection resolved to. a bind of "Left Shift"
  // deliberately answers to the right one as well, the way every other game
  // treats the pair
  std::vector<SDL_Keycode> AdditiveSelectKeys;

  float ScrollWheelY = 0.f;


};