// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ErrorReporter.h"
#include "EventQueue.h"
#include "InputDevice.h"
#include "InputEvent.h"

// reads input states, dispatches events to seperate game instances on a per
// device basis
class InputListener {
 public:
  explicit InputListener(SDL_Window* SdlWindow);

  void Update();

  void AddListenerQueue(InputDevice* DeviceToListen, EventQueue* QueueToNotify);

  InputDevice* GetDeviceFromSDLId(Sint32 ID);

  ErrorReporter InputErrorReporter;

  // map the id of the Device to the SDL device id
  // reconnecting a controller changes the id so this needs to be rechecked in
  // update
  std::unordered_map<Sint32, InputDevice*> Devices;

  std::vector<InputDevice*> GetUnintegratedDevices();

 private:
  // seperate each listening queue by their device
  std::unordered_map<InputDevice*, EventQueue*> ListeningQueues;

  // non owning pointer, Ogre owns this since i bound to the main render window
  SDL_Window* SdlWindow;
  // std::vector<InputDevice*> Device;

  // init
  void DeviceSetup();

  // Handlers
  void RemapOrCreateDevice(DeadDeviceIdError Context);

  // helpers
  bool IsAxisTrigger(SDL_Joystick* Joystick, Uint8 Axis);
  void CacheTriggerAxes(InputDevice* Device, Sint32 InstanceId);
  float NormalizeTrigger(Sint16 RawValue);
  bool CheckForDisconnectedDevices();
  SDL_Joystick* JoystickFromInstance(SDL_JoystickID Id);
  bool ControllersNeedReconnecting = false;
  int NumJoySticks;

  std::queue<Sint32> ReconnectionQueue;
  std::unordered_set<Sint32> ReconnectionSet;

  std::unordered_set<Sint32> PendingUnknownIds;

};
