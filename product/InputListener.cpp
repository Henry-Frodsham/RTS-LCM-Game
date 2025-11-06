//Copyright © 2025 Henry Frodsham
#include "InputListener.h"

InputListener::InputListener(SDL_Window* SdlWindow)
	: SdlWindow(SdlWindow)
	, InputErrorReporter(){

	// configure handlers
	InputErrorReporter.AddHandler<DeadDeviceIdError>(std::bind(&InputListener::RemapOrCreateDevice, this, std::placeholders::_1));
	
	InputListener::InitialDeviceSetup();
}

// read new key/button information
void InputListener::Update() {
	InputErrorReporter.Dispatch();
	SDL_Event Event;
	while (SDL_PollEvent(&Event)) {
		// first, class which device it is then read which key/button
		Sint32 SdlDeviceIndex = -1;

		//discern which method to use when finding the device
		switch (Event.type) {
			case SDL_CONTROLLERDEVICEADDED:
				InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_CONTOLLER_CONNECT,fmt::format("new controller with guid: {}", Event.cdevice.which)));
				//init the controller
				SDL_JoystickOpen(Event.cdevice.which);
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
				InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_CONTROLLER_DISCONNECT, fmt::format("controller with guid: {} has disconnected", Event.cdevice.which)));
				break;
			case SDL_KEYDOWN:
				// SDL2 doesnt have any device index for KBM so just use -1
				SdlDeviceIndex = -1;
				break;
			case SDL_KEYUP:
				// SDL2 doesnt have any device index for KBM so just use -1
				SdlDeviceIndex = -1;
				break;
			case SDL_CONTROLLERBUTTONDOWN:
				SdlDeviceIndex = Event.cbutton.which;
				break;
			case SDL_CONTROLLERBUTTONUP:
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
		}
		catch (const std::out_of_range& e) {
			//throws when the sdl id has changed, so pass to the handler
			InputErrorReporter.EnqueueError(DeadDeviceIdError{ SDL_JoystickOpen(Event.jdevice.which), SdlDeviceIndex, fmt::format("unrecognised device id {}", SdlDeviceIndex) });

			// exit to allow the handler to register/edit the device then revisit next event poll
			return;
		}

		EventQueue* QueueToNotify = nullptr;
		try {
			QueueToNotify = ListeningQueues.at(Device);
		}
		catch (const std::out_of_range& e) {
			InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::UNSET_INPUT_LISTENER_QUEUE, fmt::format("input device with device id {} has no listener Queue", SdlDeviceIndex)));

			//no bus so dont try and publish
			return;
		}

		switch (Event.type) {
			case SDL_KEYDOWN:
				QueueToNotify->Enqueue(RawKBEvent{ Event.key, false });
				break;

			case SDL_KEYUP:
				QueueToNotify->Enqueue(RawKBEvent{ Event.key, true });
				break;

			case SDL_CONTROLLERBUTTONDOWN:
				QueueToNotify->Enqueue(RawButtonEvent{ Event.cbutton, false });
				break;

			case SDL_CONTROLLERBUTTONUP:
				QueueToNotify->Enqueue(RawButtonEvent{ Event.cbutton, true });
				break;

			case SDL_MOUSEMOTION:
				QueueToNotify->Enqueue(RawCursorEvent{ Event.motion });
				break;

			case SDL_JOYAXISMOTION:
				QueueToNotify->Enqueue(RawAxisEvent{ Event.jaxis });
				break;
		}
	}
}

void InputListener::AddListenerQueue(InputDevice* DeviceToListen, EventQueue* QueueToNotify) {
	ListeningQueues.emplace(DeviceToListen, QueueToNotify);
}

void InputListener::RemapOrCreateDevice(DeadDeviceIdError Context) {
	// seperates by KBM and controller
	if (Context.JoyStick) {
		SDL_JoystickGUID CurGuid = SDL_JoystickGetGUID(Context.JoyStick);
		//try find an existing InputDevice with the same GUID
		for (const auto& [Key, Value] : Devices) {
			if (Value->InputType == InputDeviceType::KBM
				|| Value->InputType == InputDeviceType::NONE) {
				// skip the uninitialised or keyboard devices
				continue;
			}

			//check if GUIDs match
			//forced to compare memory since SDL2 is restrictive in terms of helper functions
			
			if (memcmp(&CurGuid, &Value->ControllerPersistentId, sizeof(SDL_JoystickGUID)) == 0) {

				Sint32 NewKey = Context.SupposedId;
				InputDevice* Device = Value;

				//delete old entry with key
				Devices.erase(Key);

				//then, replace
				Devices[NewKey] = Device;

				InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_HANDLER_ID_SUCCESS, fmt::format("succesfully relinked controller with device id: {}", NewKey)));
				return;
			}
			
		}
		//if not then create

		//alloc to the heap since its a pointer
		InputDevice* NewDevice = new InputDevice{ SDL_JoystickGetGUID(Context.JoyStick), InputDeviceType::CONTROLLER};
		Sint32 Key = Context.SupposedId;

		Devices[Key] = NewDevice;

		InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_HANDLER_NEW_REG, fmt::format("new controller with device id: {} has been succesfully registered", Key)));

	}
	else {
		// KBM has no GUID so just find the key assigned to a KBM
		for (const auto& [Key, Value] : Devices) {
			if (Value->InputType == InputDeviceType::KBM) {
				Sint32 NewKey = Context.SupposedId;
				InputDevice* Device = Value;

				//delete old entry with key
				Devices.erase(Key);
				Devices[NewKey] = Device;

				InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_HANDLER_ID_SUCCESS, fmt::format("KBM succesfully relinked to Device id: {}", NewKey)));
				return;
			}
		}
		// first KBM registration

		InputDevice* NewDevice = new InputDevice{ {0}, InputDeviceType::KBM};
		Sint32 Key = Context.SupposedId;

		Devices[Key] = NewDevice;

		InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_HANDLER_NEW_REG, fmt::format("KBM succesfully registered to Device id: {}", Key)));
	}
}

InputDevice* InputListener::GetDeviceFromSDLId(Sint32 ID) {
	try {
		return Devices.at(ID);
	}
	catch (const std::out_of_range& e) {
		InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::BAD_SDL_ID_ON_REQUEST, fmt::format("Couldnt retrieve an InputDevice from request asking for Device id: {}", ID)));
	}
}

void InputListener::InitialDeviceSetup() {
	// additional warnings
	if (SDL_NumJoysticks() < 1) {
		InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_NO_CONNECTED_CONTROLLERS));
	}

	// initialise currently plugged in controllers, if a new one is plugged in while the game is running then thats handled dynamically
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		SDL_Joystick* J = SDL_JoystickOpen(i);
		if (!J) {
			InputErrorReporter.EnqueueError(ErrorDetail::CreateError(ErrorCode::SDL_CONTROLLER_FAILED_INIT, fmt::format("controller with Device id: {} failed to init", i)));
		}

		//additionally, add these to the managed devices
		InputErrorReporter.EnqueueError(DeadDeviceIdError{ J, i });
	}

	//force KB creation at device id -1
	InputErrorReporter.EnqueueError(DeadDeviceIdError{ {0}, -1 });

}