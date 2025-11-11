//Copyright © 2025 Henry Frodsham
#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include "ErrorReporter.h"
#include "InputDevice.h"
#include "InputEvent.h"
#include "EventQueue.h"


// reads input states, dispatches events to seperate game instances on a per device basis
class InputListener {
public:
	InputListener(SDL_Window* SdlWindow);

	void Update();
	
	void AddListenerQueue(InputDevice* DeviceToListen, EventQueue* QueueToNotify);

	InputDevice* GetDeviceFromSDLId(Sint32 ID);

	ErrorReporter InputErrorReporter;

	//map the id of the Device to the SDL device id
	//reconnecting a controller changes the id so this needs to be rechecked in update
	std::unordered_map<Sint32, InputDevice*> Devices;

private:
	//seperate each listening queue by their device
	std::unordered_map<InputDevice*,EventQueue*> ListeningQueues;

	//non owning pointer, Ogre owns this since i bound to the main render window
	SDL_Window* SdlWindow;
	//std::vector<InputDevice*> Device;


	//init
	void InitialDeviceSetup();

	//Handlers
	void RemapOrCreateDevice(DeadDeviceIdError Context);



};