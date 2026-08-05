// Copyright (c) 2025 Henry Frodsham
#include "InputListener.h"

#include <vector>

InputListener::InputListener(SDL_Window* Window)
    : InputErrorReporter(), SdlWindow(Window) {
  if (SdlWindow != nullptr) {
    SdlWindowId = SDL_GetWindowID(SdlWindow);
  }

  // SDL_INIT_GAMECONTROLLER is what loads the controller mapping database.
  // RenderSystem only inits VIDEO|JOYSTICK, and without the gamecontroller
  // subsystem every SDL_GameControllerGetBindFor* call below fails, which is
  // why the trigger axes were silently never resolving. initialising a
  // subsystem twice is safe so this is defensive rather than a duplicate.
  if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0) {
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0) {
      InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
          ErrorCode::SDL_CONTROLLER_FAILED_INIT,
          fmt::format("the SDL game controller subsystem failed to init, raw "
                      "button and axis indices cannot be verified and will be "
                      "guessed. SDL reported: {}",
                      SDL_GetError())));
    }
  }

  // configure handlers
  InputErrorReporter.AddHandler<DeadDeviceIdError>(std::bind(
      &InputListener::RemapOrCreateDevice, this, std::placeholders::_1));

  DeviceSetup();
}

// read new key/button information
void InputListener::Update() {
  InputErrorReporter.Dispatch();

  if (SDL_NumJoysticks() != NumJoySticks) {
    // sdl wont do anything with controllers till theyre setup
    // so if a controller has disconnected or been reconnected then it needs to
    // be setup again
    DeviceSetup();
  }

  ControllersNeedReconnecting = CheckForDisconnectedDevices();

  SDL_Event Event;
  while (SDL_PollEvent(&Event)) {
    Sint32 SdlDeviceIndex = KBMDeviceId;
    if (!RouteEvent(Event, &SdlDeviceIndex)) {
      continue;
    }

    if (!EventBelongsToWindow(Event)) {
      continue;
    }

    // find, not at + catch. this runs for every event of every frame so the
    // exception path was both slow and noisy
    auto DeviceIt = Devices.find(SdlDeviceIndex);
    if (DeviceIt == Devices.end()) {
      if (PendingUnknownIds.insert(SdlDeviceIndex).second) {
        // first sighting only
        SDL_Joystick* J = JoystickFromInstance(SdlDeviceIndex);
        InputErrorReporter.EnqueueError(DeadDeviceIdError{
            J, SdlDeviceIndex,
            fmt::format("unrecognised device id {}", SdlDeviceIndex)});
      }
      continue;
    }

    InputDevice* Device = DeviceIt->second;
    EventQueue* QueueToNotify = FindListenerQueue(Device);
    if (QueueToNotify == nullptr) {
      // continue, not return. one unrouted device used to abandon every
      // remaining event that frame for every other device too
      continue;
    }

    PublishEvent(Event, Device, QueueToNotify);
  }
}

// decides which registered device an SDL event came from
// returns false when the event type isnt one this class forwards
bool InputListener::RouteEvent(const SDL_Event& Event, Sint32* OutDeviceId) {
  switch (Event.type) {
    // SDL2 doesnt have any device index for KBM so just use -1
    case SDL_KEYDOWN:
    case SDL_KEYUP:
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEWHEEL:
      *OutDeviceId = KBMDeviceId;
      return true;

    // jbutton, not cbutton. SDL_JOYBUTTON* events populate the jbutton member
    // of the union, reading cbutton only worked by accident because the two
    // structs happen to share a layout
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      *OutDeviceId = Event.jbutton.which;
      return true;

    case SDL_JOYAXISMOTION:
      *OutDeviceId = Event.jaxis.which;
      return true;

    default:
      return false;
  }
}

// keyboard and mouse events are window scoped, joystick events arent.
// a windowID of 0 means SDL couldnt attribute the event, so let it through
// rather than dropping input
bool InputListener::EventBelongsToWindow(const SDL_Event& Event) const {
  if (SdlWindowId == 0) {
    return true;
  }

  Uint32 EventWindow = 0;
  switch (Event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      EventWindow = Event.key.windowID;
      break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      EventWindow = Event.button.windowID;
      break;
    case SDL_MOUSEMOTION:
      EventWindow = Event.motion.windowID;
      break;
    case SDL_MOUSEWHEEL:
      EventWindow = Event.wheel.windowID;
      break;
    default:
      return true;
  }

  return EventWindow == 0 || EventWindow == SdlWindowId;
}

void InputListener::PublishEvent(const SDL_Event& Event,
                                 const InputDevice* Device,
                                 EventQueue* QueueToNotify) {
  switch (Event.type) {
    case SDL_KEYDOWN:
      QueueToNotify->Enqueue(RawKBEvent{Event.key, false});
      break;

    case SDL_KEYUP:
      QueueToNotify->Enqueue(RawKBEvent{Event.key, true});
      break;

    case SDL_JOYBUTTONDOWN:
      QueueToNotify->Enqueue(RawButtonEvent{Event.jbutton, false});
      break;

    case SDL_JOYBUTTONUP:
      QueueToNotify->Enqueue(RawButtonEvent{Event.jbutton, true});
      break;

    case SDL_MOUSEMOTION:
      QueueToNotify->Enqueue(RawCursorEvent{Event.motion});
      break;

    case SDL_MOUSEWHEEL:
      QueueToNotify->Enqueue(RawMouseWheelEvent{Event.wheel});
      break;

    case SDL_MOUSEBUTTONDOWN:
      QueueToNotify->Enqueue(RawMouseButtonEvent{Event.button, false});
      break;

    case SDL_MOUSEBUTTONUP:
      QueueToNotify->Enqueue(RawMouseButtonEvent{Event.button, true});
      break;

    // this case covers both joystick and trigger motion, hence both are
    // handled here
    case SDL_JOYAXISMOTION:
      if (IsTriggerAxis(Device, Event.jaxis.axis)) {
        QueueToNotify->Enqueue(
            RawTriggerEvent{Event.jaxis, NormalizeTrigger(Event.jaxis.value)});
      } else {
        QueueToNotify->Enqueue(RawAxisEvent{Event.jaxis});
      }
      break;

    default:
      break;
  }
}

EventQueue* InputListener::FindListenerQueue(InputDevice* Device) {
  auto QueueIt = ListeningQueues.find(Device);
  if (QueueIt != ListeningQueues.end() && QueueIt->second != nullptr) {
    return QueueIt->second;
  }

  if (ReportedMissingQueues.insert(Device).second) {
    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::UNSET_INPUT_LISTENER_QUEUE,
        fmt::format("an input device of type {} has no listener Queue, its "
                    "events are being dropped",
                    static_cast<int>(Device->InputType))));
  }
  return nullptr;
}

void InputListener::AddListenerQueue(InputDevice* DeviceToListen,
                                     EventQueue* QueueToNotify) {
  ListeningQueues[DeviceToListen] = QueueToNotify;
  ReportedMissingQueues.erase(DeviceToListen);
}

void InputListener::RemapOrCreateDevice(DeadDeviceIdError Context) {
  // seperates by KBM and controller
  if (Context.SupposedId == KBMDeviceId) {
    for (const auto& [Key, Value] : Devices) {
      if (Value->InputType == InputDeviceType::KBM) {
        // KBM has no GUID and its key never changes, so if one already exists
        // theres nothing to remap
        return;
      }
    }

    // first KBM registration
    InputDevice* NewDevice = new InputDevice(nullptr, InputDeviceType::KBM);
    Devices[KBMDeviceId] = NewDevice;
    PendingUnknownIds.erase(KBMDeviceId);

    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_HANDLER_NEW_REG,
        fmt::format("KBM succesfully registered to Device id: {}",
                    KBMDeviceId)));
    return;
  }

  if (Context.JoyStick == nullptr ||
      SDL_JoystickGetAttached(Context.JoyStick) == SDL_FALSE) {
    // if the one being relinked isnt attached then dont bother
    return;
  }

  const Sint32 NewKey = SDL_JoystickInstanceID(Context.JoyStick);

  // only add new controllers if all controllers are active
  if (!ControllersNeedReconnecting) {
    // alloc to the heap since its a pointer
    InputDevice* NewDevice =
        new InputDevice(Context.JoyStick, InputDeviceType::CONTROLLER);
    ResolveBindings(NewDevice, NewKey);
    Devices[NewKey] = NewDevice;
    PendingUnknownIds.erase(NewKey);

    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_HANDLER_NEW_REG,
        fmt::format("new controller with device id: {} has been succesfully "
                    "registered",
                    NewKey)));
    return;
  }

  const Sint32 OldKey = ReconnectionQueue.front();
  auto OldIt = Devices.find(OldKey);
  if (OldIt == Devices.end()) {
    // the slot went away underneath us, drop the request rather than wedging
    // the queue forever
    ReconnectionQueue.pop();
    ReconnectionSet.erase(OldKey);
    if (ReconnectionSet.empty()) {
      ControllersNeedReconnecting = false;
    }
    return;
  }

  // reuse the existing InputDevice rather than allocating a replacement.
  // InputTranslator::ManagedDevice, ViewPortController's registered device and
  // OverlayController's device->overlay map all hold this exact pointer, so
  // swapping in a new allocation left all of them pointing at an object that
  // no longer receives events (and leaked the old one).
  InputDevice* Device = OldIt->second;
  Device->Controller = Context.JoyStick;
  Device->InputType = InputDeviceType::CONTROLLER;
  ResolveBindings(Device, NewKey);

  Devices.erase(OldIt);
  Devices[NewKey] = Device;
  PendingUnknownIds.erase(NewKey);
  PendingUnknownIds.erase(OldKey);

  InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
      ErrorCode::SDL_HANDLER_ID_SUCCESS,
      fmt::format("successfully relinked controller, device id {} is now {}",
                  OldKey, NewKey)));

  // ListeningQueues is keyed by the device pointer, which hasnt changed, so
  // theres nothing to re-key here
  if (EventQueue* DeviceQueue = FindListenerQueue(Device)) {
    DeviceQueue->Enqueue(ReconnectControllerSuccessEvent());
  }

  ReconnectionQueue.pop();
  ReconnectionSet.erase(OldKey);

  if (ReconnectionSet.empty()) {
    ControllersNeedReconnecting = false;
  }
}

bool InputListener::CheckForDisconnectedDevices() {
  bool MarkedDeviceAsInactive = false;

  for (const auto& [Key, Value] : Devices) {
    if (Value->InputType != InputDeviceType::CONTROLLER) {
      continue;
    }

    if (SDL_JoystickGetAttached(Value->Controller) != SDL_FALSE) {
      continue;
    }

    if (!ReconnectionSet.contains(Key)) {
      ReconnectionQueue.push(Key);
      ReconnectionSet.insert(Key);

      // find, not operator[]. operator[] would insert a null queue for a
      // device that never had one, which then makes GetUnintegratedDevices
      // report it as already integrated
      if (EventQueue* DeviceQueue = FindListenerQueue(Value)) {
        DeviceQueue->Enqueue(ReconnectControllerPromptEvent(
            static_cast<int>(ReconnectionQueue.size())));
      }
    }
    MarkedDeviceAsInactive = true;
  }

  return MarkedDeviceAsInactive;
}

InputDevice* InputListener::GetDeviceFromSDLId(Sint32 ID) {
  auto DeviceIt = Devices.find(ID);
  if (DeviceIt != Devices.end()) {
    return DeviceIt->second;
  }

  InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
      ErrorCode::BAD_SDL_ID_ON_REQUEST,
      fmt::format("Couldnt retrieve an InputDevice from request asking for "
                  "Device id: {}",
                  ID)));
  // the old version fell off the end of the function here, which is undefined
  // behaviour, callers now get a null they can check
  return nullptr;
}

bool InputListener::IsTriggerAxis(const InputDevice* Device, Uint8 Axis) {
  const ControllerBindings& Binds = Device->Bindings;
  const int AxisIndex = static_cast<int>(Axis);

  if (Binds.Resolved) {
    // the pads own mapping is authoritative, no need to guess
    return AxisIndex == Binds.LeftTriggerAxis ||
           AxisIndex == Binds.RightTriggerAxis;
  }

  // no SDL mapping exists for this pad, fall back to the resting position
  // heuristic: triggers rest at the axis minimum, sticks rest centred
  Sint16 InitialState = 0;
  if (Device->Controller != nullptr &&
      SDL_JoystickGetAxisInitialState(Device->Controller, Axis,
                                      &InitialState) == SDL_TRUE) {
    return InitialState == SDL_JOYSTICK_AXIS_MIN;
  }
  return false;
}

float InputListener::NormalizeTrigger(Sint16 RawValue) {
  return (RawValue - SDL_JOYSTICK_AXIS_MIN) /
         static_cast<float>(SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN);
}

void InputListener::DeviceSetup() {
  NumJoySticks = SDL_NumJoysticks();

  // additional warnings
  if (NumJoySticks < 1) {
    InputErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::SDL_NO_CONNECTED_CONTROLLERS));
  }

  // initialise currently plugged in controllers, if a new one is plugged in
  // while the game is running then thats handled dynamically.
  // < rather than <=. the old bound ran one index past the end, so every
  // rescan tried to open a joystick that doesnt exist and logged a bogus
  // init failure for it
  for (int i = 0; i < NumJoySticks; i++) {
    if (SDL_JoystickFromInstanceID(SDL_JoystickGetDeviceInstanceID(i)) !=
        nullptr) {
      // already open, dont stack another reference on every rescan
      continue;
    }

    if (SDL_JoystickOpen(i) == nullptr) {
      InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
          ErrorCode::SDL_CONTROLLER_FAILED_INIT,
          fmt::format("controller at device index {} failed to init: {}", i,
                      SDL_GetError())));
    }
  }

  // force KB creation at device id -1 if it doesnt already exist
  if (!Devices.contains(KBMDeviceId)) {
    InputErrorReporter.EnqueueError(DeadDeviceIdError{nullptr, KBMDeviceId});
  }
}

// asks SDL what the raw joystick indices actually are for this specific pad,
// instead of assuming a layout. bindType tells us whether the control is even
// wired up the way we expect on this device
void InputListener::ResolveBindings(InputDevice* Device, Sint32 InstanceId) {
  Device->Bindings = ControllerBindings{};

  int DeviceIndex = -1;
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_JoystickGetDeviceInstanceID(i) == InstanceId) {
      DeviceIndex = i;
      break;
    }
  }

  if (DeviceIndex < 0) {
    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::BAD_SDL_ID_ON_REQUEST,
        fmt::format("no device index maps to instance id {}, its raw button "
                    "and axis indices cant be resolved",
                    InstanceId)));
    return;
  }

  if (SDL_IsGameController(DeviceIndex) == SDL_FALSE) {
    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_CONTOLLER_CONNECT,
        fmt::format("SDL has no controller mapping for \"{}\", the translator "
                    "will fall back to assumed raw indices",
                    SDL_JoystickNameForIndex(DeviceIndex) != nullptr
                        ? SDL_JoystickNameForIndex(DeviceIndex)
                        : "unknown device")));
    return;
  }

  // hold our own joystick reference across the SDL_GameControllerClose below.
  // closing the controller decrements the joysticks ref count, and if the
  // controller held the only one it would close the joystick out from under
  // Device->Controller
  SDL_JoystickOpen(DeviceIndex);

  SDL_GameController* GameCtrl = SDL_GameControllerOpen(DeviceIndex);
  if (GameCtrl == nullptr) {
    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_CONTROLLER_FAILED_INIT,
        fmt::format("could not open device index {} as a game controller: {}",
                    DeviceIndex, SDL_GetError())));
    return;
  }

  auto RawAxis = [GameCtrl](SDL_GameControllerAxis Axis) {
    SDL_GameControllerButtonBind Bind =
        SDL_GameControllerGetBindForAxis(GameCtrl, Axis);
    return Bind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS ? Bind.value.axis : -1;
  };

  auto RawButton = [GameCtrl](SDL_GameControllerButton Button) {
    SDL_GameControllerButtonBind Bind =
        SDL_GameControllerGetBindForButton(GameCtrl, Button);
    return Bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON ? Bind.value.button
                                                           : -1;
  };

  ControllerBindings& Binds = Device->Bindings;
  Binds.LeftTriggerAxis = RawAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  Binds.RightTriggerAxis = RawAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
  Binds.LeftStickXAxis = RawAxis(SDL_CONTROLLER_AXIS_LEFTX);
  Binds.LeftStickYAxis = RawAxis(SDL_CONTROLLER_AXIS_LEFTY);
  Binds.RightStickXAxis = RawAxis(SDL_CONTROLLER_AXIS_RIGHTX);
  Binds.RightStickYAxis = RawAxis(SDL_CONTROLLER_AXIS_RIGHTY);

  Binds.FaceButtonA = RawButton(SDL_CONTROLLER_BUTTON_A);
  Binds.FaceButtonB = RawButton(SDL_CONTROLLER_BUTTON_B);
  Binds.FaceButtonX = RawButton(SDL_CONTROLLER_BUTTON_X);
  Binds.FaceButtonY = RawButton(SDL_CONTROLLER_BUTTON_Y);
  Binds.LeftShoulder = RawButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
  Binds.RightShoulder = RawButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
  Binds.Start = RawButton(SDL_CONTROLLER_BUTTON_START);

  Binds.Resolved = true;

  SDL_GameControllerClose(GameCtrl);

  InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
      ErrorCode::SDL_CONTOLLER_CONNECT,
      fmt::format("resolved raw indices for instance {} - A:{} B:{} X:{} Y:{} "
                  "LT axis:{} RT axis:{} LX axis:{} LY axis:{}",
                  InstanceId, Binds.FaceButtonA, Binds.FaceButtonB,
                  Binds.FaceButtonX, Binds.FaceButtonY, Binds.LeftTriggerAxis,
                  Binds.RightTriggerAxis, Binds.LeftStickXAxis,
                  Binds.LeftStickYAxis)));
}

SDL_Joystick* InputListener::JoystickFromInstance(SDL_JoystickID Id) {
  if (SDL_Joystick* J = SDL_JoystickFromInstanceID(Id)) {
    return J;  // already open (DeviceSetup opened it)
  }
  // fallback: not opened yet - find its device index and open it
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_JoystickGetDeviceInstanceID(i) == Id) {  // SDL 2.0.6+
      return SDL_JoystickOpen(i);
    }
  }
  return nullptr;
}

std::vector<InputDevice*> InputListener::GetUnintegratedDevices() {
  std::vector<InputDevice*> DeviceList;

  for (const auto& [Key, Value] : Devices) {
    auto QueueIt = ListeningQueues.find(Value);
    if (QueueIt == ListeningQueues.end() || QueueIt->second == nullptr) {
      DeviceList.push_back(Value);
    }
  }

  return DeviceList;
}