// Copyright © 2025 Henry Frodsham
#include "InputListener.h"

InputListener::InputListener(SDL_Window* SdlWindow)
    : SdlWindow(SdlWindow), InputErrorReporter() {
  // configure handlers
  InputErrorReporter.AddHandler<DeadDeviceIdError>(std::bind(
      &InputListener::RemapOrCreateDevice, this, std::placeholders::_1));

  InputListener::DeviceSetup();
}

// read new key/button information
void InputListener::Update() {
  InputErrorReporter.Dispatch();
  SDL_Event Event;

  if (SDL_NumJoysticks() != NumJoySticks) {
    // sdl wont do anything with controllers till theyre setup
    // so if a controller has disconnected or been reconnected then it needs to
    // be setup again
    DeviceSetup();
  }

  while (SDL_PollEvent(&Event)) {
    // first, class which device it is then read which key/button
    Sint32 SdlDeviceIndex = -1;

    // discern which method to use when finding the device
    switch (Event.type) {
      case SDL_CONTROLLERDEVICEADDED:
        InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
            ErrorCode::SDL_CONTOLLER_CONNECT,
            fmt::format("new controller with guid: {}", Event.cdevice.which)));
        // init the controller
        SDL_JoystickOpen(Event.cdevice.which);
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
            ErrorCode::SDL_CONTROLLER_DISCONNECT,
            fmt::format("controller with guid: {} has disconnected",
                        Event.cdevice.which)));
        break;
      case SDL_KEYDOWN:
        // SDL2 doesnt have any device index for KBM so just use -1
        SdlDeviceIndex = -1;
        break;
      case SDL_KEYUP:
        // SDL2 doesnt have any device index for KBM so just use -1
        SdlDeviceIndex = -1;
        break;
      case SDL_MOUSEBUTTONDOWN:
        SdlDeviceIndex = -1; 
        break;
      case SDL_MOUSEBUTTONUP:
        SdlDeviceIndex = -1;
        break;
      case SDL_JOYBUTTONDOWN:
        SdlDeviceIndex = Event.cbutton.which;
        break;
      case SDL_JOYBUTTONUP:
        SdlDeviceIndex = Event.cbutton.which;
        break;
      case SDL_MOUSEMOTION:
        // for simplicity, just keep KB -1 and mouse -2
        SdlDeviceIndex = -1;
        break;
      case SDL_JOYAXISMOTION:
        SdlDeviceIndex = Event.jaxis.which;
        break;
      default:
        continue;
    }

    InputDevice* Device = nullptr;
    try {
      Device = Devices.at(SdlDeviceIndex);
    } catch (const std::out_of_range& e) {
      // throws when the sdl id has changed, so pass to the handler
      InputErrorReporter.EnqueueError(DeadDeviceIdError{
          SDL_JoystickOpen(SdlDeviceIndex), SdlDeviceIndex,
          fmt::format("unrecognised device id {}", SdlDeviceIndex)});

      // exit to allow the handler to register/edit the device then revisit next
      // event poll
      return;
    }

    EventQueue* QueueToNotify = nullptr;
    try {
      QueueToNotify = ListeningQueues.at(Device);
    } catch (const std::out_of_range& e) {
      InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
          ErrorCode::UNSET_INPUT_LISTENER_QUEUE,
          fmt::format("input device with device id {} has no listener Queue",
                      SdlDeviceIndex)));

      // no bus so dont try and publish
      return;
    }

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
      //this case covers both joystick and trigger motion, hence both are handled here
      case SDL_JOYAXISMOTION:
        if (IsAxisTrigger(Device->Controller, Event.jaxis.axis)) {
          QueueToNotify->Enqueue(RawTriggerEvent{
              Event.jaxis, NormalizeTrigger(Event.jaxis.value)});
        } else {
          QueueToNotify->Enqueue(RawAxisEvent{Event.jaxis});
        }
        break;
      case SDL_MOUSEBUTTONDOWN:
        QueueToNotify->Enqueue(RawMouseButtonEvent{Event.button, false});
        break;
      case SDL_MOUSEBUTTONUP:
        QueueToNotify->Enqueue(RawMouseButtonEvent{Event.button, true});
        break;
    }
  }
}

void InputListener::AddListenerQueue(InputDevice* DeviceToListen,
                                     EventQueue* QueueToNotify) {
  ListeningQueues.emplace(DeviceToListen, QueueToNotify);
}

void InputListener::RemapOrCreateDevice(DeadDeviceIdError Context) {
  // seperates by KBM and controller
  if (Context.SupposedId != -1) {
    SDL_bool NewAttached = SDL_JoystickGetAttached(Context.JoyStick);
    if (NewAttached == SDL_FALSE) {
      // if the one being relinked isnt attached then dont bother
      return;
    }

    for (auto it = Devices.begin(); it != Devices.end();) {
      Sint32 Key = it->first;
      InputDevice* Value = it->second;

      if (Value->InputType == InputDeviceType::KBM ||
          Value->InputType == InputDeviceType::NONE) {
        ++it;  // Only increment if not erasing
        continue;
      }

      // non ideal temporary solution
      // theres a well acknowledge issue with sdl where generic controllers cant
      // be distinguished therefore, just check if theres a controller slot that
      // isnt plugged in then reattach to that
      SDL_bool Attached = SDL_JoystickGetAttached(Value->Controller);
      if (Attached == SDL_FALSE) {
        // dont just trust SupposedId
        // regrab the active instance id from sdl
        Sint32 NewKey = SDL_JoystickInstanceID(Context.JoyStick);
        InputDevice* Device =
            new InputDevice(Context.JoyStick, InputDeviceType::CONTROLLER);
        CacheTriggerAxes(Device, NewKey);
        EventQueue* OldQueue = ListeningQueues[Value];

        it = Devices.erase(it);

        Devices.emplace(NewKey, Device);

        ListeningQueues.erase(Value);
        ListeningQueues.emplace(Device, OldQueue);

        InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
            ErrorCode::SDL_HANDLER_ID_SUCCESS,
            fmt::format("successfully relinked controller with device id: {}",
                        NewKey)));
        return;
      } else {
        ++it;
      }
    }
    // if not then create

    // alloc to the heap since its a pointer
    InputDevice* NewDevice =
        new InputDevice{Context.JoyStick, InputDeviceType::CONTROLLER};

    Sint32 Key = Context.SupposedId;
    CacheTriggerAxes(NewDevice, Key);
    Devices[Key] = NewDevice;

    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_HANDLER_NEW_REG,
        fmt::format(
            "new controller with device id: {} has been succesfully registered",
            Key)));

  } else {
    // KBM has no GUID so just find the key assigned to a KBM
    for (const auto& [Key, Value] : Devices) {
      if (Value->InputType == InputDeviceType::KBM) {
        Sint32 NewKey = Context.SupposedId;
        InputDevice* Device = Value;

        // delete old entry with key
        Devices.erase(Key);
        Devices[NewKey] = Device;

        InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
            ErrorCode::SDL_HANDLER_ID_SUCCESS,
            fmt::format("KBM succesfully relinked to Device id: {}", NewKey)));
        return;
      }
    }
    // first KBM registration

    InputDevice* NewDevice = new InputDevice({0}, InputDeviceType::KBM);
    Sint32 Key = Context.SupposedId;

    Devices[Key] = NewDevice;

    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::SDL_HANDLER_NEW_REG,
        fmt::format("KBM succesfully registered to Device id: {}", Key)));
  }
}

InputDevice* InputListener::GetDeviceFromSDLId(Sint32 ID) {
  try {
    return Devices.at(ID);
  } catch (const std::out_of_range& e) {
    InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
        ErrorCode::BAD_SDL_ID_ON_REQUEST,
        fmt::format("Couldnt retrieve an InputDevice from request asking for "
                    "Device id: {}",
                    ID)));
  }
}
bool InputListener::IsAxisTrigger(SDL_Joystick* Joystick, Uint8 Axis) {
  Sint16 InitialState = 0;
  if (SDL_JoystickGetAxisInitialState(Joystick, Axis, &InitialState)) {
    return InitialState == SDL_JOYSTICK_AXIS_MIN;
  }
  return false;
}

float InputListener::NormalizeTrigger(Sint16 RawValue) {
  return (RawValue - SDL_JOYSTICK_AXIS_MIN) /
         static_cast<float>(SDL_JOYSTICK_AXIS_MAX - SDL_JOYSTICK_AXIS_MIN);
}

void InputListener::DeviceSetup() {
  // additional warnings
  if (SDL_NumJoysticks() < 1) {
    InputErrorReporter.EnqueueError(
        ErrorDetail::CreateError(ErrorCode::SDL_NO_CONNECTED_CONTROLLERS));
  }

  // initialise currently plugged in controllers, if a new one is plugged in
  // while the game is running then thats handled dynamically
  NumJoySticks = SDL_NumJoysticks();
  for (int i = 0; i < NumJoySticks; i++) {
    SDL_Joystick* J = SDL_JoystickOpen(i);
    if (!J) {
      InputErrorReporter.EnqueueError(ErrorDetail::CreateError(
          ErrorCode::SDL_CONTROLLER_FAILED_INIT,
          fmt::format("controller with Device id: {} failed to init", i)));
    }

    Sint32 InstanceId = SDL_JoystickInstanceID(J);

    if (Devices.contains(InstanceId)) {
      continue;
    }

    // additionally, add these to the managed devices
    InputErrorReporter.EnqueueError(DeadDeviceIdError{J, InstanceId});
  }

  // force KB creation at device id -1 if it doesnt already exist
  if (!Devices.contains(-1)) {
    InputErrorReporter.EnqueueError(DeadDeviceIdError{nullptr, -1});
  }
}
void InputListener::CacheTriggerAxes(InputDevice* Device, Sint32 InstanceId) {
  for (int i = 0; i < SDL_NumJoysticks(); i++) {
    if (SDL_JoystickGetDeviceInstanceID(i) != InstanceId) {
      continue;
    }
    if (!SDL_IsGameController(i)) {
      return;
    }
    SDL_GameController* GC = SDL_GameControllerOpen(i);
    if (!GC) {
      return;
    }
    SDL_GameControllerButtonBind LeftBind =
        SDL_GameControllerGetBindForAxis(GC, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    SDL_GameControllerButtonBind RightBind =
        SDL_GameControllerGetBindForAxis(GC, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    if (LeftBind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS) {
      Device->LeftTriggerRawAxis = LeftBind.value.axis;
    }
    if (RightBind.bindType == SDL_CONTROLLER_BINDTYPE_AXIS) {
      Device->RightTriggerRawAxis = RightBind.value.axis;
    }
    SDL_GameControllerClose(GC);
    return;
  }
}
std::vector<InputDevice*> InputListener::GetUnintegratedDevices() {
  std::vector<InputDevice*> DeviceList;
  // return early to prevent iterating over the devices unnecessarily
  if (Devices.size() == ListeningQueues.size()) {
    return DeviceList;
  }

  for (const auto& [Key, Value] : Devices) {
    if (!ListeningQueues.contains(Value)) {
      DeviceList.push_back(Value);
    }
  }

  return DeviceList;
}
