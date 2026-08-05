// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>

#include <queue>
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
  explicit InputListener(SDL_Window* Window);

  void Update();

  void AddListenerQueue(InputDevice* DeviceToListen, EventQueue* QueueToNotify);

  // returns nullptr when the id isnt registered, callers must check
  InputDevice* GetDeviceFromSDLId(Sint32 ID);

  std::vector<InputDevice*> GetUnintegratedDevices();

  ErrorReporter InputErrorReporter;

  // map the id of the Device to the SDL device id
  // reconnecting a controller changes the id so this needs to be rechecked in
  // update
  std::unordered_map<Sint32, InputDevice*> Devices;

 private:
  // SDL2 has no instance id for keyboard/mouse, so the pair is pinned here
  static constexpr Sint32 KBMDeviceId = -1;

  // init
  void DeviceSetup();

  // Handlers
  void RemapOrCreateDevice(DeadDeviceIdError Context);

  // event routing, split out of Update so the poll loop stays readable
  // returns false for event types this class doesnt care about
  static bool RouteEvent(const SDL_Event& Event, Sint32* OutDeviceId);
  bool EventBelongsToWindow(const SDL_Event& Event) const;
  void PublishEvent(const SDL_Event& Event, const InputDevice* Device,
                    EventQueue* QueueToNotify);

  EventQueue* FindListenerQueue(InputDevice* Device);

  // reads the SDL_GameController mapping for a pad and caches the *actual* raw
  // joystick indices onto the device
  void ResolveBindings(InputDevice* Device, Sint32 InstanceId);

  // helpers
  static bool IsTriggerAxis(const InputDevice* Device, Uint8 Axis);
  static float NormalizeTrigger(Sint16 RawValue);
  static SDL_Joystick* JoystickFromInstance(SDL_JoystickID Id);
  bool CheckForDisconnectedDevices();

  // seperate each listening queue by their device
  std::unordered_map<InputDevice*, EventQueue*> ListeningQueues;

  // non owning pointer, Ogre owns this since i bound to the main render window
  SDL_Window* SdlWindow;
  Uint32 SdlWindowId = 0;

  bool ControllersNeedReconnecting = false;
  int NumJoySticks = 0;

  std::queue<Sint32> ReconnectionQueue;
  std::unordered_set<Sint32> ReconnectionSet;

  std::unordered_set<Sint32> PendingUnknownIds;

  // an unrouted device would otherwise log an error for every single event it
  // produces, which is thousands a second for a moving mouse
  std::unordered_set<InputDevice*> ReportedMissingQueues;
};