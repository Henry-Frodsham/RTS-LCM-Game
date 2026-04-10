// Copyright (c) 2025 Henry Frodsham
#include "InputTranslator.h"

#include <vector>

#include "InputAnalyser.h"
#include "SharedInputEvent.h"
InputTranslator::InputTranslator(InputDevice* Device, float VPWidth,
                                 float VPHeight, int ThreadNum,
                                 std::vector<float> SDim) {
  ManagedDevice = Device;

  InputEvents = new EventBus();
  WaitingEvents = new EventQueue(InputEvents);
  ActionBus = new EventBus();
  ActionQueue = new EventQueue(ActionBus);

  CursorPos = {0.f, 0.f};
  JoyStickStates = {0.f, 0.f};
  InputEvents->Subscribe<RawKBEvent>(
      std::bind(&InputTranslator::TranslateRawKB, this, std::placeholders::_1));
  InputEvents->Subscribe<RawButtonEvent>(std::bind(
      &InputTranslator::TranslateRawButton, this, std::placeholders::_1));
  InputEvents->Subscribe<RawCursorEvent>(std::bind(
      &InputTranslator::TranslateRawCursor, this, std::placeholders::_1));
  InputEvents->Subscribe<RawAxisEvent>(std::bind(
      &InputTranslator::TranslateRawAxis, this, std::placeholders::_1));
  InputEvents->Subscribe<RawMouseButtonEvent>(std::bind(
      &InputTranslator::TranslateRawMouseButton, this, std::placeholders::_1));
  InputEvents->Subscribe<RawTriggerEvent>(std::bind(
      &InputTranslator::TranslateRawTriggerEvent, this, std::placeholders::_1));
  CursorSensitivity = 1500.f;
  JoystickDeadzone = 0.1f;

  ThreadNumber = ThreadNum;

  ViewPortWidth = VPWidth;
  ViewPortHeight = VPHeight;
  ScreenDimensions = SDim;
  TranslationErrorReporter = new ErrorReporter();

#ifdef _DEBUG
  InputAnalyser::GetInstance().RegisterNew(this);
#endif
}

bool InputTranslator::HasAction(GameAction Action) {
  return ActiveActions.contains(Action);
}
ActionContext InputTranslator::GetActionContext(GameAction Action) {
  try {
    return ActiveActions.at(Action);
  } catch (std::out_of_range e) {
    TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::INVALID_ORDER_OF_OPERATIONS,
        fmt::format("Action context could not be provided because the action "
                    "does not exist")));
  }
}

bool InputTranslator::getKeyState(char Key) { return KeyStates.contains(Key); }

void InputTranslator::TranslateRawKB(RawKBEvent Event) {
  SDL_Keycode Key = Event.Key.keysym.sym;

  if (Key == SDLK_e) {
    if (Event.KeyUp) {
      ActiveActions.erase(GameAction::USE);
    } else if (!ActiveActions.contains(GameAction::USE)) {
      ActiveActions.emplace(GameAction::USE,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(UseActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      // using at is safe since prev selection eliminates risk of ActiveActions
      // not containing the action (and no risk of another thread causing issues
      // since this class is thread specific)
      ActionContext& Context = ActiveActions.at(GameAction::USE);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }

  } else if (Key == SDLK_q) {
    if (Event.KeyUp) {
      ActiveActions.erase(GameAction::BACK);
    } else if (!ActiveActions.contains(GameAction::BACK)) {
      ActiveActions.emplace(GameAction::BACK,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(BackActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      ActionContext& Context = ActiveActions.at(GameAction::BACK);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }
  } else if (Key == SDLK_r) {
    if (Event.KeyUp) {
      ActiveActions.erase(GameAction::CONTXT);
    } else if (!ActiveActions.contains(GameAction::CONTXT)) {
      ActiveActions.emplace(GameAction::CONTXT,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(ContextActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      ActionContext& Context = ActiveActions.at(GameAction::CONTXT);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }
  }

  // sdl2 not case sensitive by default so check the state of shift and capslock
  Uint16 modifiers = Event.Key.keysym.mod;

  if (Key >= 32 && Key <= 126) {
    char C = static_cast<char>(Key);

    // check state of shift and capslock
    // if pressed then convert to upper case
    if (C >= 'a' && C <= 'z' &&
        ((modifiers & KMOD_SHIFT) || (modifiers & KMOD_CAPS))) {
      C = C - 32;
    }

    // only store the pressed keys so remove if keys been released
    if (Event.KeyUp) {
      KeyStates.erase(C);
    } else {
      KeyStates.insert(C);
    }
  }
}
void InputTranslator::TranslateRawMouseButton(RawMouseButtonEvent Event) {
  int ButtonIndex = -1;
  if (Event.Button.button == SDL_BUTTON_LEFT) {
    ButtonIndex = 0;
  } else if (Event.Button.button == SDL_BUTTON_RIGHT) {
    ButtonIndex = 1;
  } else if (Event.Button.button == SDL_BUTTON_MIDDLE) {
    ButtonIndex = 2;
  } else {
    TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::UNRECOGNISED,
        fmt::format("Whilst Translating a mouse button event, the pressed "
                    "button has been corrupted. this indicates a corrupted SDL "
                    "DLL or device driver")));
    return;
  }
  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> RelativeCoordinates = std::vector<float>{
      CursorPos[0] / ViewPortWidth, CursorPos[1] / ViewPortHeight};
  if (!Event.Released) {
    if (!MouseButtonStates[ButtonIndex]) {
      MouseButtonStates[ButtonIndex] = true;
      // button was just pressed so send the event
      if (ButtonIndex == 0) {
        PressActionCommand Action = PressActionCommand(
            ActionContext(RelativeCoordinates[0], RelativeCoordinates[1], true,
                          ThreadNumber, ManagedDevice),
            false);
        RS.RenderQueue->Enqueue(Action);
        ActionQueue->Enqueue(Action);
      }
    }

  } else {
    if (MouseButtonStates[ButtonIndex]) {
      MouseButtonStates[ButtonIndex] = false;
      // button was just released so send the event
      if (ButtonIndex == 0) {
        PressActionCommand Action = PressActionCommand(
            ActionContext(RelativeCoordinates[0], RelativeCoordinates[1], true,
                          ThreadNumber, ManagedDevice),
            true);
        RS.RenderQueue->Enqueue(Action);
        ActionQueue->Enqueue(Action);
      }
    }
  }
}
void InputTranslator::TranslateRawTriggerEvent(RawTriggerEvent Event) {
  Uint8 IncomingAxis = Event.AxisEvent.axis;
  float NormalizedValue = Event.NormalizedValue;

  // use normalised value with a threshold
  static constexpr float TriggerThreshold = 0.1f;
  bool Pressed = (NormalizedValue > TriggerThreshold);

  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> RelativeCoordinates = std::vector<float>{
      CursorPos[0] / ViewPortWidth, CursorPos[1] / ViewPortHeight};

  if (ManagedDevice->RightTriggerRawAxis >= 0 &&
      IncomingAxis == ManagedDevice->RightTriggerRawAxis) {
    if (Pressed && !TriggerStates[1]) {
      TriggerStates[1] = true;
      PressActionCommand Action = PressActionCommand(
          ActionContext(RelativeCoordinates[0], RelativeCoordinates[1], true,
                        ThreadNumber, ManagedDevice),
          false);
      RS.RenderQueue->Enqueue(Action);
      ActionQueue->Enqueue(Action);

    } else if (!Pressed && TriggerStates[1]) {
      TriggerStates[1] = false;
      PressActionCommand Action = PressActionCommand(
          ActionContext(RelativeCoordinates[0], RelativeCoordinates[1], true,
                        ThreadNumber, ManagedDevice),
          false);
      RS.RenderQueue->Enqueue(Action);
      ActionQueue->Enqueue(Action);
    }
  } else if (ManagedDevice->LeftTriggerRawAxis >= 0 &&
             IncomingAxis == ManagedDevice->LeftTriggerRawAxis) {
    if (Pressed && !TriggerStates[0]) {
      TriggerStates[0] = true;
    } else if (!Pressed && TriggerStates[0]) {
      TriggerStates[0] = false;
    }
  }
}
void InputTranslator::TranslateRawButton(RawButtonEvent Event) {
  Uint8 ButtonIndex = Event.Button.button;

  if (ButtonIndex == 0) {
    if (Event.ButtonUp) {
      ActiveActions.erase(GameAction::USE);
    } else if (!ActiveActions.contains(GameAction::USE)) {
      ActiveActions.emplace(GameAction::USE,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(UseActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      ActionContext& Context = ActiveActions.at(GameAction::USE);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }
  } else if (ButtonIndex == 1) {
    if (Event.ButtonUp) {
      ActiveActions.erase(GameAction::BACK);
    } else if (!ActiveActions.contains(GameAction::BACK)) {
      ActiveActions.emplace(GameAction::BACK,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(BackActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      ActionContext& Context = ActiveActions.at(GameAction::BACK);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }
  } else if (ButtonIndex == 2) {
    if (Event.ButtonUp) {
      ActiveActions.erase(GameAction::CONTXT);
    } else if (!ActiveActions.contains(GameAction::CONTXT)) {
      ActiveActions.emplace(GameAction::CONTXT,
                            ActionContext(CursorPos[0], CursorPos[1], true,
                                          ThreadNumber, ManagedDevice));
      ActionQueue->Enqueue(ContextActionCommand(ActionContext(
          CursorPos[0], CursorPos[1], true, ThreadNumber, ManagedDevice)));
    } else {
      ActionContext& Context = ActiveActions.at(GameAction::CONTXT);
      Context.JustPressed = false;
      Context.MouseX = CursorPos[0];
      Context.MouseY = CursorPos[1];
    }
  }

  if (Event.ButtonUp) {
    ButtonStates.erase(Event.Button.button);
  } else {
    ButtonStates.insert(Event.Button.button);
  }
}
void InputTranslator::TranslateRawCursor(RawCursorEvent Event) {
  const SDL_MouseMotionEvent& Motion = Event.Cursor;
  std::vector<float> CursorVec{static_cast<float>(Motion.x),
                               static_cast<float>(Motion.y)};
  CursorVec[0] = std::clamp(CursorVec[0], 0.f, ViewPortWidth);
  CursorVec[1] = std::clamp(CursorVec[1], 0.f, ViewPortHeight);
  RelativeMotion = Ogre::Vector2f((CursorVec[0] - CursorPos[0]) / 100.f,
                                  (CursorVec[1] - CursorPos[1]) / 100.f);

  CursorPos = CursorVec;

  // notify interested classes (like overlayController) on movement events
  // since we cant have direct access of the mouse position from other threads
  // (classes running in this thread can do that just fine though)
  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> RelativeCoordinates = std::vector<float>{
      CursorVec[0] / ViewPortWidth, CursorVec[1] / ViewPortHeight};
  RS.RenderQueue->Enqueue(CursorMovementEvent(CursorVec, RelativeCoordinates,
                                              ThreadNumber, ManagedDevice));
}
void InputTranslator::TranslateRawAxis(RawAxisEvent Event) {
  const SDL_JoyAxisEvent& SDL_Ev = Event.Axis;

  float NormalizedValue = static_cast<float>(SDL_Ev.value) / 32767.0f;

  NormalizedValue = std::clamp(NormalizedValue, -1.0f, 1.0f);

  // Left stick X axis
  if (SDL_Ev.axis == 0) {
    JoyStickStates[0] = NormalizedValue;
  } else if (SDL_Ev.axis == 1) {
    JoyStickStates[1] = NormalizedValue;
  }

  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> RelativeCoordinates = std::vector<float>{
      CursorPos[0] / ViewPortWidth, CursorPos[1] / ViewPortHeight};
  RS.RenderQueue->Enqueue(CursorMovementEvent(CursorPos, RelativeCoordinates,
                                              ThreadNumber, ManagedDevice));
}

int InputTranslator::GetNumPressedKeys() {
  if (ManagedDevice->InputType == InputDeviceType::KBM) {
    return KeyStates.size();
  } else {
    return ButtonStates.size();
  }
}

std::vector<float> InputTranslator::GetCurrentAxis() { return CursorPos; }

// converts a joystick float to one that takes into account the deadzone
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
  // reset relative motion before events are processed, so if theres a tick
  // without motion then its cleared
  RelativeMotion = Ogre::Vector2f(0.f, 0.f);
  WaitingEvents->Dispatch();
  ActionQueue->Dispatch();
  if (ManagedDevice->InputType == InputDeviceType::CONTROLLER) {
    float stickX = ApplyDeadzone(JoyStickStates[0], JoystickDeadzone);
    float stickY = ApplyDeadzone(JoyStickStates[1], JoystickDeadzone);

    float velocityX = stickX * CursorSensitivity * DeltaTime;
    float velocityY = stickY * CursorSensitivity * DeltaTime;

    // update and clamp both axis
    CursorPos[0] += velocityX;
    CursorPos[1] += velocityY;

    RelativeMotion = Ogre::Vector2f(velocityX / 100.f, velocityY / 100.f);
  }
  CursorPos[0] = std::clamp(CursorPos[0], 0.f, ViewPortWidth);
  CursorPos[1] = std::clamp(CursorPos[1], 0.f, ViewPortHeight);

  TranslationErrorReporter->Dispatch();
}

Ogre::Vector2f InputTranslator::GetRelativeMotion() { return RelativeMotion; }
bool InputTranslator::HasRelativeMotion() {
  if (RelativeMotion != Ogre::Vector2f(0.f, 0.f)) {
    return true;
  }
  return false;
}
bool InputTranslator::HoldingRMBorLT() {
  return MouseButtonStates[1] || TriggerStates[0];
}

std::vector<float> InputTranslator::GetViewPortDimensions() {
  return std::vector<float>{ViewPortWidth, ViewPortHeight};
}
void InputTranslator::ResizeViewPortDimensions(ResizedViewPortEvent Event) {
  ViewPortWidth = Event.X;
  ViewPortHeight = Event.Y;
}
std::vector<float> InputTranslator::GetScreenDimensions() {
  return ScreenDimensions;
}
