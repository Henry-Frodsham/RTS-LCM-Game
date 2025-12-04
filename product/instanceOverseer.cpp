//Copyright © 2025 Henry Frodsham
#include "instanceOverseer.h"

InstanceOverseer::InstanceOverseer(InputListener* ParentListener)
	:DeviceListener(ParentListener){
	InstanceBus = new EventBus();
	InstanceQueue = new EventQueue(InstanceBus);
	InstanceReporter = new ErrorReporter();
	InstanceBus->Subscribe<RegisterInstanceEvent>(std::bind(&InstanceOverseer::RegisterNewInstance, this, std::placeholders::_1));
}

void InstanceOverseer::RegisterNewInstance(RegisterInstanceEvent Event) {
	if (!Event.InstanceDevice) {
		InstanceReporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::INSTANCE_REQUEST_NO_DEVICE));
		return;
	}

	InputTranslator* Translator = new InputTranslator(Event.InstanceDevice);
	DeviceListener->AddListenerQueue(Event.InstanceDevice, Translator->WaitingEvents);
	GameInstance* NewInstance = new GameInstance(InstanceReporter, InstanceQueue, Event.InstanceDevice, Translator);

	GameInstances.push_back(NewInstance);

	RenderSystem& RS = RenderSystem::GetInstance();

	//KBM should always have control of the main viewport for text prompts etc
	//if the new device isnt the kbm then just make a new one
	if (Event.InstanceDevice->InputType == InputDeviceType::KBM) {
		InstanceViewports.emplace(NewInstance,
			RS.GetPrimaryViewport());
	}
	else {
		InstanceViewports.emplace(NewInstance,
			RS.CreateViewPort());
	}
}

void InstanceOverseer::ReviseAndUpdate(float DeltaTime) {
	InstanceQueue->Dispatch();
	InstanceReporter->Dispatch();

	std::vector<InputDevice*> NewInstanceDevices = DeviceListener->GetUnintegratedDevices();
	if (NewInstanceDevices.size()) {
		for (InputDevice* D : NewInstanceDevices) {
			InstanceQueue->Enqueue(RegisterInstanceEvent(D));
		}
	}

	std::vector<std::thread> Threads;
	for (GameInstance* Instance : GameInstances) {
		Threads.emplace_back(&GameInstance::Run, Instance, DeltaTime);
	}

	for (auto& Thread : Threads) {
		Thread.join();
	}
}