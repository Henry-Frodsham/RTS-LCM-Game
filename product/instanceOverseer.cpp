//Copyright © 2025 Henry Frodsham
#include "instanceOverseer.h"

InstanceOverseer::InstanceOverseer(InputListener* ParentListener)
	:DeviceListener(ParentListener){
	InstanceBus = new EventBus();
	InstanceQueue = new EventQueue(InstanceBus);
	InstanceReporter = new ErrorReporter();
	InstanceBus->Subscribe<RegisterInstanceEvent>(std::bind(&InstanceOverseer::RegisterNewInstance, this, std::placeholders::_1));
	InstanceBus->Subscribe<UpstreamOrbitViewport2DEvent>(std::bind(&InstanceOverseer::MoveViewport2DOrbit, this, std::placeholders::_1));
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

void InstanceOverseer::MoveViewport2DOrbit(UpstreamOrbitViewport2DEvent Event) {
	ViewPortController* ViewPortToMove = nullptr;
	try {
		ViewPortToMove = InstanceViewports.at(Event.InstanceRequesting);
	}
	catch (std::exception e){
		InstanceReporter->EnqueueError(ErrorDetail::CreateError(ErrorCode::BAD_VIEWPORT_CONTROL_REQUEST, 
			fmt::format("a game instance was not associated with any viewport to be controlled")));
		return;
	}
	//temporary demonstration using a fixed orbit point
	//in future this will be set in the event
	ViewPortToMove->MoveCameraOrbitingPoint2DMotion(Event.RelativeMotion, Ogre::Vector3f(0.5f, 0.f, -5.f));
}