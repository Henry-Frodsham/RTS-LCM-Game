//Copyright © 2025 Henry Frodsham
#include "InputListener.h"

InputListener::InputListener(SDL_Window* SdlWindow)
	: SdlWindow(SdlWindow)
	, InputErrorReporter(){

	// configure handlers
	InputErrorReporter.AddHandler<DeadDeviceIdError>(std::bind(&InputListener::RemapOrCreateDevice, this, std::placeholders::_1));

}

// read new key/button information
void InputListener::update() {
	InputErrorReporter.Dispatch();
	SDL_Event Event;
	while (SDL_PollEvent(&Event)) {
		// first, class which device it is then read which key/button
		Sint32 SdlDeviceIndex = Event.cdevice.which;

		InputDevice* Device = nullptr;
		try {
			Device = Devices.at(SdlDeviceIndex);
		}
		catch (const std::out_of_range& e) {
			//throws when the sdl id has changed, so pass to the handler
			InputErrorReporter.EnqueueError(DeadDeviceIdError{ SDL_JoystickOpen(Event.jdevice.which), SdlDeviceIndex });

			// exit to allow the handler to register/edit the device then revisit next event poll
			return;
		}

		EventQueue* QueueToNotify = nullptr;
		try {
			QueueToNotify = ListeningQueues.at(Device);
		}
		catch (const std::out_of_range& e) {
			InputErrorReporter.EnqueueError(ErrorDetail::ErrorManifest.at(ErrorCode::UNSET_INPUT_LISTENER_QUEUE));

			//no bus so dont try and publish
			return;
		}

		switch (Event.type) {
			case SDL_KEYDOWN:
				QueueToNotify->Enqueue(RawKBEvent{ Event.key });
				break;

			case SDL_CONTROLLERBUTTONDOWN:
				QueueToNotify->Enqueue(RawButtonEvent{ Event.cbutton });
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

void InputListener::addListenerQueue(InputDevice* DeviceToListen, EventQueue* QueueToNotify) {
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

				InputErrorReporter.EnqueueError(ErrorDetail::ErrorManifest.at(ErrorCode::SDL_HANDLER_ID_SUCCESS));
				return;
			}
			
		}
		//if not then create

		//alloc to the heap since its a pointer
		InputDevice* NewDevice = new InputDevice{ SDL_JoystickGetGUID(Context.JoyStick), InputDeviceType::CONTROLLER};
		Sint32 Key = Context.SupposedId;

		Devices[Key] = NewDevice;

		InputErrorReporter.EnqueueError(ErrorDetail::ErrorManifest.at(ErrorCode::SDL_HANDLER_NEW_REG));

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

				InputErrorReporter.EnqueueError(ErrorDetail::ErrorManifest.at(ErrorCode::SDL_HANDLER_ID_SUCCESS));
				return;
			}
		}
		// first KBM registration

		InputDevice* NewDevice = new InputDevice{ {0}, InputDeviceType::KBM};
		Sint32 Key = Context.SupposedId;

		Devices[Key] = NewDevice;

		InputErrorReporter.EnqueueError(ErrorDetail::ErrorManifest.at(ErrorCode::SDL_HANDLER_NEW_REG));
	}
}