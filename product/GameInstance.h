#pragma once
#include "EventQueue.h"
#include "EventBus.h"
#include "InputListener.h"
#include "ErrorReporter.h"
#include "InputTranslator.h"

class GameInstance {
public:
	GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ, InputDevice* Device, InputTranslator* DeviceTranslator);

	void Run(float DT);
	EventQueue* LocalQueue;

	InputDevice* InstanceDevice;
private:
	ErrorReporter* ParentReporter;

	//this is ok to access directly since only this instance will access the translator
	InputTranslator* InstanceTranslator;
	//for non thread safe requests such as accessing RenderSystem
	//ensures all non thread safe commands from instances are read in serial and not in parallel
	EventQueue* UpstreamQueue;

	//thread safe commands only accessing local resources
	EventBus* LocalBus;
};