// Copyright (c) 2025 Henry Frodsham
#include "InputTranslator.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "RenderSystem.h"
#include "SharedInputEvent.h"

#ifdef _DEBUG
#include "InputAnalyser.h"
#endif

InputTranslator::InputTranslator(InputDevice* Device, float VPWidth,
                                 float VPHeight, int ThreadNum,
                                 std::vector<float> SDim)
    : ManagedDevice(Device),
      ScreenDimensions(SDim),
      ViewPortWidth(VPWidth),
      ViewPortHeight(VPHeight),
      ThreadNumber(ThreadNum) {
  TranslationErrorReporter = new ErrorReporter();

  InputEvents = new EventBus();
  WaitingEvents = new EventQueue(InputEvents);
  ActionBus = new EventBus();
  ActionQueue = new EventQueue(ActionBus);

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
  InputEvents->Subscribe<RawMouseWheelEvent>(
      std::bind(&InputTranslator::TranslateRawMouseWheelEvent, this,
                std::placeholders::_1));
  InputEvents->Subscribe<ReconnectControllerPromptEvent>(std::bind(
      &InputTranslator::HandleReconnectPrompt, this, std::placeholders::_1));
  InputEvents->Subscribe<ReconnectControllerSuccessEvent>(std::bind(
      &InputTranslator::HideReconnectPrompt, this, std::placeholders::_1));

  InputEvents->Subscribe<ConfigAppliedEvent>(std::bind(
      &InputTranslator::ReloadConfiguration, this, std::placeholders::_1));

  InputConfig = new ConfigManager("InputSettings", TranslationErrorReporter,
                                  std::to_string(ThreadNumber));

  LoadConfiguration();
  BuildKeyBindings();
  RebuildPadButtonBindings();
  CreateReconnectPrompt();

#ifdef _DEBUG
  InputAnalyser::GetInstance().RegisterNew(this);
#endif
}

InputTranslator::~InputTranslator() {
  delete ActionQueue;
  delete ActionBus;
  delete WaitingEvents;
  delete InputEvents;
  delete InputConfig;
  delete TranslationErrorReporter;
}

// sensitivity, deadzones and the prompt layout were all hardcoded in the
// constructor. they live in config/default/InputSettings.json now, and each
// instance can override them with config/custom/InputSettings<N>.json
void InputTranslator::LoadConfiguration() {
  CursorSensitivity =
      InputConfig->GetValueOrDefault<float>("CursorSensitivity");
  ControllerOrbitSensitivity =
      InputConfig->GetValueOrDefault<float>("ControllerOrbitSensitivity");
  JoystickDeadzone = InputConfig->GetValueOrDefault<float>("JoystickDeadzone");
  TriggerThreshold = InputConfig->GetValueOrDefault<float>("TriggerThreshold");
  RelativeMotionScale =
      InputConfig->GetValueOrDefault<float>("RelativeMotionScale");
  RightDragThresholdPixels =
      InputConfig->GetValueOrDefault<float>("RightDragThresholdPixels");
  SelectDragThresholdPixels =
      InputConfig->GetValueOrDefault<float>("SelectDragThresholdPixels");
  RightClickOpensContextWheel =
      InputConfig->GetValueOrDefault<bool>("RightClickOpensContextWheel");
  // LB/RB "held button" zoom speed, expressed as scroll wheel notches per
  // second so it lines up with GetMouseWheelY. see the ZOOM CONTROL comment
  // in InputTranslator.h
  ShoulderZoomNotchesPerSecond =
      InputConfig->GetValueOrDefault<float>("ShoulderZoomNotchesPerSecond");

  PromptPosition =
      InputConfig->GetValueOrDefault<std::vector<float>>("PromptPosition");
  PromptDimensions =
      InputConfig->GetValueOrDefault<std::vector<float>>("PromptDimensions");
  PromptBorderMaterial =
      InputConfig->GetValueOrDefault<std::string>("PromptBorderMaterial");
  PromptTextMaterial =
      InputConfig->GetValueOrDefault<std::string>("PromptTextMaterial");

  // a missing or malformed key returns a default constructed value, which for
  // a divisor or a coordinate pair would be quietly catastrophic
  if (RelativeMotionScale <= 0.f) {
    RelativeMotionScale = 100.f;
  }
  if (ControllerOrbitSensitivity <= 0.f) {
    ControllerOrbitSensitivity = 1.2f;
  }
  if (ShoulderZoomNotchesPerSecond <= 0.f) {
    ShoulderZoomNotchesPerSecond = 6.f;
  }
  // a zero here would make every click a box drag over the whole viewport
  if (SelectDragThresholdPixels <= 0.f) {
    SelectDragThresholdPixels = 6.f;
  }
  if (PromptPosition.size() < 2) {
    PromptPosition = {0.2f, 0.3f};
  }
  if (PromptDimensions.size() < 2) {
    PromptDimensions = {0.24f, 0.03f};
  }
  if (PromptBorderMaterial.empty()) {
    PromptBorderMaterial = "RED";
  }
  if (PromptTextMaterial.empty()) {
    PromptTextMaterial = "WHITE";
  }
}

// an options page has written this instance's settings file. every value read
// in LoadConfiguration is a cached copy of what was in that file, so all of
// them are stale and all of them are read again - along with the bindings and
// the prompt, which are built out of the same file
//
// this arrives on the input bus rather than being called directly, because the
// page runs on the application thread and this object runs on its instance's.
// GameInstance puts it on the queue that Update drains, so by the time it gets
// here it is on the one thread that owns these members
void InputTranslator::ReloadConfiguration(ConfigAppliedEvent Event) {
  // every instance's file is announced the same way, so a page belonging to
  // somebody else's split screen is not this translator's business
  if (Event.ConfigName != "InputSettings" ||
      Event.InstanceName != std::to_string(ThreadNumber)) {
    return;
  }

  InputConfig->LoadOrReload();

  LoadConfiguration();
  BuildKeyBindings();
  RefreshReconnectPrompt();
}

// key names are the SDL names ("E", "Q", "Space", "Left Shift"), see
// SDL_GetKeyName for the exact spelling of anything unusual
void InputTranslator::BuildKeyBindings() {
  KeyBindings.clear();

  const std::pair<const char*, GameAction> Wanted[] = {
      {"KeyBindUse", GameAction::USE},
      {"KeyBindBack", GameAction::BACK},
      {"KeyBindContext", GameAction::CONTXT}};

  for (const auto& [ConfigKey, Action] : Wanted) {
    const std::string KeyName =
        InputConfig->GetValueOrDefault<std::string>(ConfigKey);
    const SDL_Keycode Code = SDL_GetKeyFromName(KeyName.c_str());

    if (Code == SDLK_UNKNOWN) {
      TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
          ErrorCode::UNRECOGNISED,
          fmt::format("\"{}\" is not a key SDL recognises, the binding for {} "
                      "has been dropped",
                      KeyName, ConfigKey)));
      continue;
    }

    KeyBindings[Code] = Action;
  }

  // the add-to-selection modifier isnt a GameAction - it has no command of its
  // own, it only colours the meaning of a select gesture - so it is resolved
  // here into its own latch rather than going through KeyBindings
  AdditiveSelectKeys.clear();
  AdditiveSelectKeyHeld = false;

  const std::string ModifierName =
      InputConfig->GetValueOrDefault<std::string>("KeyBindAddToSelection");
  const SDL_Keycode ModifierCode = SDL_GetKeyFromName(ModifierName.c_str());

  if (ModifierCode == SDLK_UNKNOWN) {
    TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::UNRECOGNISED,
        fmt::format("\"{}\" is not a key SDL recognises, add-to-selection has "
                    "fallen back to either shift key",
                    ModifierName)));
    AdditiveSelectKeys = {SDLK_LSHIFT, SDLK_RSHIFT};
    return;
  }

  AdditiveSelectKeys.push_back(ModifierCode);

  // binding one shift binds the other. nobody means "the left one only" when
  // they write "Left Shift", and the same goes for ctrl and alt
  switch (ModifierCode) {
    case SDLK_LSHIFT: AdditiveSelectKeys.push_back(SDLK_RSHIFT); break;
    case SDLK_RSHIFT: AdditiveSelectKeys.push_back(SDLK_LSHIFT); break;
    case SDLK_LCTRL: AdditiveSelectKeys.push_back(SDLK_RCTRL); break;
    case SDLK_RCTRL: AdditiveSelectKeys.push_back(SDLK_LCTRL); break;
    case SDLK_LALT: AdditiveSelectKeys.push_back(SDLK_RALT); break;
    case SDLK_RALT: AdditiveSelectKeys.push_back(SDLK_LALT); break;
    default: break;
  }
}

// the old code assumed raw pad button 0 was A, 1 was B and 2 was X. thats only
// true for some drivers. InputListener has already asked SDL for the real
// indices, so use them and only guess when SDL genuinely has no mapping
void InputTranslator::RebuildPadButtonBindings() {
  PadButtonBindings.clear();

  if (ManagedDevice == nullptr ||
      ManagedDevice->InputType != InputDeviceType::CONTROLLER) {
    return;
  }

  const ControllerBindings& Binds = ManagedDevice->Bindings;

  if (!Binds.Resolved) {
    TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::UNRECOGNISED,
        fmt::format("device {} has no SDL controller mapping, falling back to "
                    "assumed raw button indices 0/1/2 for USE/BACK/CONTXT",
                    ThreadNumber)));
    PadButtonBindings[0] = GameAction::USE;
    PadButtonBindings[1] = GameAction::BACK;
    PadButtonBindings[2] = GameAction::CONTXT;
    return;
  }

  if (Binds.FaceButtonA >= 0) {
    PadButtonBindings[Binds.FaceButtonA] = GameAction::USE;
  }
  if (Binds.FaceButtonB >= 0) {
    PadButtonBindings[Binds.FaceButtonB] = GameAction::BACK;
  }
  if (Binds.FaceButtonX >= 0) {
    PadButtonBindings[Binds.FaceButtonX] = GameAction::CONTXT;
  }
}

std::string InputTranslator::PromptOverlayName() const {
  return "PROMPT_OVERLAY_" + std::to_string(ThreadNumber);
}
std::string InputTranslator::PromptBorderName() const {
  return "PROMPT_BORDER_" + std::to_string(ThreadNumber);
}
std::string InputTranslator::PromptTextName() const {
  return "PROMPT_TEXT_" + std::to_string(ThreadNumber);
}

void InputTranslator::CreateReconnectPrompt() {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Rs.RenderQueue->Enqueue(
      CreateOverlayEvent(PromptOverlayName(), ManagedDevice));

  Rs.RenderQueue->Enqueue(
      OverlayAddBoxEvent(PromptPosition, PromptDimensions, PromptBorderName(),
                         PromptBorderMaterial, PromptOverlayName()));

  Rs.RenderQueue->Enqueue(
      OverlayAddTextToPanelEvent{PromptBorderName(),
                                 PromptTextName(),
                                 "DEVICE " + std::to_string(ThreadNumber) +
                                     " press any key, queue pos: 0 ",
                                 PromptTextMaterial,
                                 {0.f, 0.f},
                                 {1.f, 1.f}});

  Rs.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      PromptOverlayName(), PromptBorderName(), false));
}

// the prompt is already on screen by the time its layout can be changed, so a
// reload edits the elements rather than making them again - creating them a
// second time would collide with the names the first set still holds
void InputTranslator::RefreshReconnectPrompt() {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Rs.RenderQueue->Enqueue(
      OverlayEditPanelEvent(PromptBorderName(), PromptOverlayName(),
                            PromptDimensions, PromptPosition,
                            PromptBorderMaterial));

  Rs.RenderQueue->Enqueue(OverlayEditTextEvent(PromptTextName(),
                                               PromptOverlayName(),
                                               {-1.f, -1.f}, {-1.f, -1.f},
                                               PromptTextMaterial));
}

bool InputTranslator::HasAction(GameAction Action) {
  return ActiveActions.contains(Action);
}

ActionContext InputTranslator::GetActionContext(GameAction Action) {
  auto Found = ActiveActions.find(Action);
  if (Found != ActiveActions.end()) {
    return Found->second;
  }

  TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
      ErrorCode::INVALID_ORDER_OF_OPERATIONS,
      fmt::format("Action context could not be provided because action {} is "
                  "not currently active",
                  static_cast<int>(Action))));

  // the old version fell off the end of the function on this path, which is
  // undefined behaviour. an inert context is checkable via JustPressed
  return ActionContext(0.f, 0.f, false, ThreadNumber, ManagedDevice);
}

bool InputTranslator::getKeyState(char Key) { return KeyStates.contains(Key); }

// every consumer reads MouseX/MouseY in normalised viewport space:
// Ogre::OverlayElement::findElementAt and Camera::getCameraToViewportRay both
// want 0-1. the keyboard and pad paths used to hand over raw pixels here
ActionContext InputTranslator::MakeContext(bool JustPressed) const {
  const float X = (ViewPortWidth > 0.f) ? CursorPos[0] / ViewPortWidth : 0.f;
  const float Y = (ViewPortHeight > 0.f) ? CursorPos[1] / ViewPortHeight : 0.f;
  return ActionContext(X, Y, JustPressed, ThreadNumber, ManagedDevice);
}

void InputTranslator::ApplyDigitalAction(GameAction Action, bool Pressed) {
  if (!Pressed) {
    ActiveActions.erase(Action);
    return;
  }

  auto Existing = ActiveActions.find(Action);
  if (Existing == ActiveActions.end()) {
    const ActionContext Context = MakeContext(true);
    ActiveActions.emplace(Action, Context);
    PublishActionCommand(Action, Context);
    return;
  }

  // already held, this is a key repeat or a second down for the same press.
  // refresh the position but stop reporting it as new
  const ActionContext Refreshed = MakeContext(false);
  Existing->second.JustPressed = false;
  Existing->second.MouseX = Refreshed.MouseX;
  Existing->second.MouseY = Refreshed.MouseY;
}

void InputTranslator::PublishActionCommand(GameAction Action,
                                           const ActionContext& Context) {
  switch (Action) {
    case GameAction::USE:
      ActionQueue->Enqueue(UseActionCommand(Context));
      break;
    case GameAction::BACK:
      ActionQueue->Enqueue(BackActionCommand(Context));
      break;
    case GameAction::CONTXT:
      ActionQueue->Enqueue(ContextActionCommand(Context));
      break;
    default:
      TranslationErrorReporter->EnqueueError(ErrorDetail::CreateError(
          ErrorCode::UNRECOGNISED,
          fmt::format("no command exists for game action {}",
                      static_cast<int>(Action))));
      break;
  }
}

// left mouse and the right trigger are the same verb, so they share one path.
// the render queue copy is what drives OverlayController::OverlayPressedCheck
void InputTranslator::PublishPressCommand(bool Released) {
  RenderSystem& RS = RenderSystem::GetInstance();
  const PressActionCommand Action(MakeContext(!Released), Released);
  RS.RenderQueue->Enqueue(Action);
  ActionQueue->Enqueue(Action);
}

// ---- select gesture ----------------------------------------------------
// opened by the select verb going down (left mouse, right trigger). the
// origin is where it went down, and the modifier is latched here so a shift
// let go of halfway through a drag cant turn a replace into an add
void InputTranslator::BeginSelectDrag() {
  SelectDragOrigin = CursorPos;
  SelectDragActive = true;
  SelectDragExceeded = false;
  SelectDragAdditive = HoldingAdditiveSelect();

  PublishSelectDragCommand(DragPhase::Begin);
}

// the cursor moved. only meaningful while a gesture is open, and only worth
// publishing once it has become a box - a click that wobbles a pixel is
// still a click and nothing downstream needs to hear about it
void InputTranslator::UpdateSelectDrag() {
  if (!SelectDragActive) {
    return;
  }

  if (!SelectDragExceeded) {
    const float DX = CursorPos[0] - SelectDragOrigin[0];
    const float DY = CursorPos[1] - SelectDragOrigin[1];
    if ((DX * DX + DY * DY) <=
        SelectDragThresholdPixels * SelectDragThresholdPixels) {
      return;
    }
    SelectDragExceeded = true;
  }

  PublishSelectDragCommand(DragPhase::Update);
}

// closed by the verb coming back up. the End phase is published whether or
// not the gesture ever became a box, because thats the edge a plain click
// acts on
void InputTranslator::EndSelectDrag(bool Cancelled) {
  if (!SelectDragActive) {
    return;
  }

  PublishSelectDragCommand(Cancelled ? DragPhase::Cancel : DragPhase::End);

  SelectDragActive = false;
  SelectDragExceeded = false;
  SelectDragAdditive = false;
}

void InputTranslator::PublishSelectDragCommand(DragPhase Phase) {
  const float OriginX =
      (ViewPortWidth > 0.f) ? SelectDragOrigin[0] / ViewPortWidth : 0.f;
  const float OriginY =
      (ViewPortHeight > 0.f) ? SelectDragOrigin[1] / ViewPortHeight : 0.f;

  ActionQueue->Enqueue(DragSelectCommand(
      MakeContext(Phase == DragPhase::Begin), OriginX, OriginY, Phase,
      SelectDragAdditive, SelectDragExceeded));
}

void InputTranslator::TranslateRawKB(RawKBEvent Event) {
  const SDL_Keycode Key = Event.Key.keysym.sym;

  auto Bound = KeyBindings.find(Key);
  if (Bound != KeyBindings.end()) {
    ApplyDigitalAction(Bound->second, !Event.KeyUp);
  }

  // held modifier, not an action - see the AdditiveSelectKeys comment in
  // BuildKeyBindings. SDL repeats a held key, so this is written as an
  // assignment rather than a counter
  if (std::find(AdditiveSelectKeys.begin(), AdditiveSelectKeys.end(), Key) !=
      AdditiveSelectKeys.end()) {
    AdditiveSelectKeyHeld = !Event.KeyUp;
  }

  // sdl2 not case sensitive by default so check the state of shift and capslock
  const Uint16 Modifiers = Event.Key.keysym.mod;

  if (Key >= 32 && Key <= 126) {
    char C = static_cast<char>(Key);

    // check state of shift and capslock
    // if pressed then convert to upper case
    if (C >= 'a' && C <= 'z' &&
        ((Modifiers & KMOD_SHIFT) || (Modifiers & KMOD_CAPS))) {
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
  const Uint8 Button = Event.Button.button;

  // indexed directly by the SDL constant, so LEFT/MIDDLE/RIGHT can never be
  // shuffled by a remapping step. some X11 setups report side buttons as 8/9
  // rather than SDL_BUTTON_X1/X2, those simply arent bound
  if (Button == 0 || Button >= MouseButtonCount) {
    return;
  }

  const bool Pressed = !Event.Released;
  if (MouseButtonStates[Button] == Pressed) {
    // SDL can repeat a state, only act on genuine edges
    return;
  }
  MouseButtonStates[Button] = Pressed;

  switch (Button) {
    case SDL_BUTTON_LEFT:
      // select / interact, mirrors the right trigger on a pad. the press
      // command is what the overlay hit test reads; the select gesture is
      // what the world reads
      PublishPressCommand(Event.Released);
      if (Pressed) {
        BeginSelectDrag();
      } else {
        EndSelectDrag();
      }
      break;

    case SDL_BUTTON_RIGHT:
      // right mouse is the camera orbit modifier (HoldingRMBorLT), so a right
      // *click* only counts as a context action when the cursor stayed put.
      // a right drag orbits and opens nothing
      if (Pressed) {
        RightDragOrigin = CursorPos;
        RightButtonDragged = false;
      } else if (RightClickOpensContextWheel && !RightButtonDragged) {
        PublishActionCommand(GameAction::CONTXT, MakeContext(true));
      }
      break;

    default:
      // middle and the side buttons are tracked for IsMouseButtonDown but
      // arent bound to an action yet
      break;
  }
}

void InputTranslator::TranslateRawTriggerEvent(RawTriggerEvent Event) {
  if (ManagedDevice == nullptr) {
    return;
  }

  const int IncomingAxis = static_cast<int>(Event.AxisEvent.axis);
  const bool Pressed = (Event.NormalizedValue > TriggerThreshold);
  const ControllerBindings& Binds = ManagedDevice->Bindings;

  if (Binds.RightTriggerAxis >= 0 && IncomingAxis == Binds.RightTriggerAxis) {
    if (Pressed != TriggerStates[RightTriggerSlot]) {
      TriggerStates[RightTriggerSlot] = Pressed;
      // Released is !Pressed. the old code passed false on both edges, so a
      // trigger release arrived at OverlayController as a second press
      PublishPressCommand(!Pressed);
      if (Pressed) {
        BeginSelectDrag();
      } else {
        EndSelectDrag();
      }
    }
    return;
  }

  if (Binds.LeftTriggerAxis >= 0 && IncomingAxis == Binds.LeftTriggerAxis) {
    // left trigger is a held modifier only, see HoldingRMBorLT
    TriggerStates[LeftTriggerSlot] = Pressed;
  }
}

void InputTranslator::TranslateRawButton(RawButtonEvent Event) {
  const Uint8 RawIndex = Event.Button.button;

  auto Bound = PadButtonBindings.find(static_cast<int>(RawIndex));
  if (Bound != PadButtonBindings.end()) {
    ApplyDigitalAction(Bound->second, !Event.ButtonUp);
  }

  if (Event.ButtonUp) {
    ButtonStates.erase(RawIndex);
  } else {
    ButtonStates.insert(RawIndex);
  }
}

void InputTranslator::TranslateRawCursor(RawCursorEvent Event) {
  const SDL_MouseMotionEvent& Motion = Event.Cursor;

  std::vector<float> CursorVec{static_cast<float>(Motion.x),
                               static_cast<float>(Motion.y)};
  CursorVec[0] = std::clamp(CursorVec[0], 0.f, ViewPortWidth);
  CursorVec[1] = std::clamp(CursorVec[1], 0.f, ViewPortHeight);

  RelativeMotion =
      Ogre::Vector2f((CursorVec[0] - CursorPos[0]) / RelativeMotionScale,
                     (CursorVec[1] - CursorPos[1]) / RelativeMotionScale);

  CursorPos = CursorVec;

  if (MouseButtonStates[SDL_BUTTON_RIGHT] && !RightButtonDragged) {
    const float DX = CursorPos[0] - RightDragOrigin[0];
    const float DY = CursorPos[1] - RightDragOrigin[1];
    if ((DX * DX + DY * DY) >
        RightDragThresholdPixels * RightDragThresholdPixels) {
      RightButtonDragged = true;
    }
  }

  // notify interested classes (like overlayController) on movement events
  // since we cant have direct access of the mouse position from other threads
  // (classes running in this thread can do that just fine though)
  RenderSystem& RS = RenderSystem::GetInstance();
  const ActionContext Normalised = MakeContext(false);
  const std::vector<float> RelativeCoordinates{Normalised.MouseX,
                                               Normalised.MouseY};
  RS.RenderQueue->Enqueue(CursorMovementEvent(CursorVec, RelativeCoordinates,
                                              ThreadNumber, ManagedDevice));

  // while the button is held, re-run the overlay hit test at the new
  // position on every move so a held UI element (e.g. a slider being
  // dragged) keeps updating instead of only reacting to the initial press.
  // deliberately RenderQueue-only, not ActionQueue - nothing on the world
  // side wants a press repeated on every move frame. the world side of a
  // held drag is the select gesture below, which is a different command
  // precisely so the two can never be confused for one another
  if (MouseButtonStates[SDL_BUTTON_LEFT]) {
    RS.RenderQueue->Enqueue(PressActionCommand(Normalised, false));
  }

  UpdateSelectDrag();
}

void InputTranslator::TranslateRawAxis(RawAxisEvent Event) {
  if (ManagedDevice == nullptr) {
    return;
  }

  const SDL_JoyAxisEvent& SDL_Ev = Event.Axis;
  const float NormalizedValue =
      std::clamp(static_cast<float>(SDL_Ev.value) / 32767.0f, -1.0f, 1.0f);

  const ControllerBindings& Binds = ManagedDevice->Bindings;
  // fall back to 0/1 only when SDL genuinely has no mapping for the pad
  const int StickX = Binds.Resolved ? Binds.LeftStickXAxis : 0;
  const int StickY = Binds.Resolved ? Binds.LeftStickYAxis : 1;
  const int IncomingAxis = static_cast<int>(SDL_Ev.axis);

  if (StickX >= 0 && IncomingAxis == StickX) {
    JoyStickStates[0] = NormalizedValue;
  } else if (StickY >= 0 && IncomingAxis == StickY) {
    JoyStickStates[1] = NormalizedValue;
  }

  // the cursor movement notification used to be sent from here with the *old*
  // cursor position, before Update had integrated the stick. its sent from
  // Update now, with the position it actually moved to
}

// ZOOM CONTROL: this is the "real" half of the zoom input, see the comment
// above GetMouseWheelY in InputTranslator.h. LB/RB (InputTranslator::Update)
// feed the exact same ScrollWheelY member so nothing downstream needs to
// know there are two ways to trigger a zoom notch
void InputTranslator::TranslateRawMouseWheelEvent(RawMouseWheelEvent Event) {
  // preciseY is SDL 2.0.18+, it falls back to the integer value on hardware
  // that cant report fractions of a notch
  const float Delta = (Event.WheelEvent.direction == SDL_MOUSEWHEEL_FLIPPED)
                          ? -Event.WheelEvent.preciseY
                          : Event.WheelEvent.preciseY;

  // accumulate. more than one wheel event can land in a single frame and a
  // plain assignment threw all but the last one away
  ScrollWheelY += Delta;
}

int InputTranslator::GetNumPressedKeys() {
  if (ManagedDevice != nullptr &&
      ManagedDevice->InputType == InputDeviceType::KBM) {
    return static_cast<int>(KeyStates.size());
  }
  return static_cast<int>(ButtonStates.size());
}

std::vector<float> InputTranslator::GetCurrentAxis() { return CursorPos; }

// converts a joystick float to one that takes into account the deadzone
float InputTranslator::ApplyDeadzone(float Value, float Deadzone) const {
  // ignore movements smaller than the deadzone
  if (std::abs(Value) < Deadzone) {
    return 0.0f;
  }

  // velocity scaling starts at the end of the deadzone
  // stops a sudden accelleration when leaving deadzone
  const float Sign = (Value > 0.0f) ? 1.0f : -1.0f;
  return Sign * ((std::abs(Value) - Deadzone) / (1.0f - Deadzone));
}

void InputTranslator::HandleReconnectPrompt(
    ReconnectControllerPromptEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Rs.RenderQueue->Enqueue(OverlayEditTextEvent(
      PromptTextName(), PromptOverlayName(), {-1.f, -1.f}, {-1.f, -1.f},
      "USE_OLD",
      "DEVICE " + std::to_string(ThreadNumber) + " press any key, queue pos: " +
          std::to_string(Event.PositionInQueue)));

  Rs.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      PromptOverlayName(), PromptBorderName(), true));
}

void InputTranslator::HideReconnectPrompt(
    ReconnectControllerSuccessEvent Event) {
  // the pad that came back may not be the model that left, so the raw indices
  // have to be re-read before any of its buttons are trusted again
  RebuildPadButtonBindings();

  ActiveActions.clear();
  ButtonStates.clear();
  TriggerStates.fill(false);
  JoyStickStates = {0.f, 0.f};

  // the trigger that opened a select gesture is never going to report its
  // release, so close it here rather than leaving a rubber band on screen.
  // cancelled rather than ended - the player never let go of anything, so
  // there is no order in it to carry out
  EndSelectDrag(/*Cancelled=*/true);

  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      PromptOverlayName(), PromptBorderName(), false));
}

void InputTranslator::Update(float DeltaTime) {
  // reset relative motion before events are processed, so if theres a tick
  // without motion then its cleared
  RelativeMotion = Ogre::Vector2f(0.f, 0.f);
  ScrollWheelY = 0.f;

  WaitingEvents->Dispatch();
  ActionQueue->Dispatch();

  if (IsShoulderHeld(/*LeftShoulder=*/true)) {
    ScrollWheelY -= ShoulderZoomNotchesPerSecond * DeltaTime;
  }
  if (IsShoulderHeld(/*LeftShoulder=*/false)) {
    ScrollWheelY += ShoulderZoomNotchesPerSecond * DeltaTime;
  }

  if (ManagedDevice != nullptr &&
      ManagedDevice->InputType == InputDeviceType::CONTROLLER) {
    const float StickX = ApplyDeadzone(JoyStickStates[0], JoystickDeadzone);
    const float StickY = ApplyDeadzone(JoyStickStates[1], JoystickDeadzone);

    // cursor ICON movement - unrelated to orbit, kept exactly as before
    const float VelocityX = StickX * CursorSensitivity * DeltaTime;
    const float VelocityY = StickY * CursorSensitivity * DeltaTime;

    // update and clamp both axis
    CursorPos[0] = std::clamp(CursorPos[0] + VelocityX, 0.f, ViewPortWidth);
    CursorPos[1] = std::clamp(CursorPos[1] + VelocityY, 0.f, ViewPortHeight);

    if (VelocityX != 0.f || VelocityY != 0.f) {
      RenderSystem& RS = RenderSystem::GetInstance();
      const ActionContext Normalised = MakeContext(false);
      const std::vector<float> RelativeCoordinates{Normalised.MouseX,
                                                   Normalised.MouseY};
      RS.RenderQueue->Enqueue(CursorMovementEvent(
          CursorPos, RelativeCoordinates, ThreadNumber, ManagedDevice));

      // same continuous-update rationale as TranslateRawCursor's mouse path,
      // for a held right trigger dragging the stick across a UI element
      if (TriggerStates[RightTriggerSlot]) {
        RS.RenderQueue->Enqueue(PressActionCommand(Normalised, false));
      }

      UpdateSelectDrag();
    }

    // orbit motion - driven directly by stick deflection at a fixed
    // radians/sec rate, deliberately independent of CursorSensitivity /
    // RelativeMotionScale above (see ControllerOrbitSensitivity comment in
    // InputTranslator.h for why those were the wrong source for this)
    if (StickX != 0.f || StickY != 0.f) {
      RelativeMotion =
          Ogre::Vector2f(StickX * ControllerOrbitSensitivity * DeltaTime,
                         StickY * ControllerOrbitSensitivity * DeltaTime);
    }
  }

  CursorPos[0] = std::clamp(CursorPos[0], 0.f, ViewPortWidth);
  CursorPos[1] = std::clamp(CursorPos[1], 0.f, ViewPortHeight);

  TranslationErrorReporter->Dispatch();
}

Ogre::Vector2f InputTranslator::GetRelativeMotion() { return RelativeMotion; }

bool InputTranslator::HasRelativeMotion() {
  return RelativeMotion != Ogre::Vector2f(0.f, 0.f);
}

bool InputTranslator::IsMouseButtonDown(Uint8 SdlButton) const {
  if (SdlButton == 0 || SdlButton >= MouseButtonCount) {
    return false;
  }
  return MouseButtonStates[SdlButton];
}

bool InputTranslator::HoldingRMBorLT() const {
  return MouseButtonStates[SDL_BUTTON_RIGHT] || TriggerStates[LeftTriggerSlot];
}

// shift on a keyboard, Y on a pad. Y is read straight out of ButtonStates for
// the same reason LB/RB are (see IsShoulderHeld) - every raw button that is
// down is tracked there whether or not a GameAction claims it, and nothing
// claims Y
bool InputTranslator::HoldingAdditiveSelect() const {
  if (AdditiveSelectKeyHeld) {
    return true;
  }

  if (ManagedDevice == nullptr ||
      ManagedDevice->InputType != InputDeviceType::CONTROLLER) {
    return false;
  }

  const int FaceY = ManagedDevice->Bindings.FaceButtonY;
  if (FaceY < 0) {
    return false;
  }

  return ButtonStates.contains(static_cast<Uint8>(FaceY));
}

// ButtonStates tracks every raw pad button thats currently down regardless of
// whether its bound to a GameAction (see TranslateRawButton), so LB/RB can be
// read here even though neither is in PadButtonBindings
bool InputTranslator::IsShoulderHeld(bool LeftShoulder) const {
  if (ManagedDevice == nullptr ||
      ManagedDevice->InputType != InputDeviceType::CONTROLLER) {
    return false;
  }

  const ControllerBindings& Binds = ManagedDevice->Bindings;
  const int Index = LeftShoulder ? Binds.LeftShoulder : Binds.RightShoulder;
  if (Index < 0) {
    // this pad's mapping doesnt expose a shoulder button, nothing to read
    return false;
  }

  return ButtonStates.contains(static_cast<Uint8>(Index));
}

int InputTranslator::GetMouseWheelY() {
  return static_cast<int>(std::lround(ScrollWheelY));
}

float InputTranslator::GetPreciseMouseWheelY() const { return ScrollWheelY; }

std::vector<float> InputTranslator::GetViewPortDimensions() {
  return std::vector<float>{ViewPortWidth, ViewPortHeight};
}

void InputTranslator::ResizeViewPortDimensions(ResizedViewPortEvent Event) {
  // keep the cursor where it was proportionally rather than letting it jump
  if (ViewPortWidth > 0.f && ViewPortHeight > 0.f && Event.X > 0.f &&
      Event.Y > 0.f) {
    CursorPos[0] = (CursorPos[0] / ViewPortWidth) * Event.X;
    CursorPos[1] = (CursorPos[1] / ViewPortHeight) * Event.Y;
  }

  ViewPortWidth = Event.X;
  ViewPortHeight = Event.Y;
}

std::vector<float> InputTranslator::GetScreenDimensions() {
  return ScreenDimensions;
}