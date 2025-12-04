//Copyright © 2025 Henry Frodsham
#pragma once
#include "GameInstance.h"
#include "EventQueue.h"
#include "EventBus.h"
#include "InputListener.h"
#include "InstanceEvent.h"
#include "ErrorReporter.h"
#include "InputTranslator.h"
#include "ViewPortController.h"
#include "RenderSystem.h"
#include <unordered_map>
#include <thread>
#include <vector>

//the class that "holds the reigns" over all instances

class InstanceOverseer {
public:
	InstanceOverseer(InputListener* ParentListener);
	EventQueue* InstanceQueue;
	void ReviseAndUpdate(float DeltaTime);
private:
	std::vector<GameInstance*> GameInstances;

	//viewports need to be stored 
	std::unordered_map<GameInstance*, ViewPortController*> InstanceViewports;

	EventBus* InstanceBus;

	ErrorReporter* InstanceReporter;

	void RegisterNewInstance(RegisterInstanceEvent Event);

	
	//a new instance is made solely based on a new device being connected
	InputListener* DeviceListener;
};