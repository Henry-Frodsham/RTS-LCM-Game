#pragma once
#include "InputDevice.h"
#include "GameInstance.h"
#include <OGRE/Ogre.h>

struct RegisterInstanceEvent {
	InputDevice* InstanceDevice;
};

//for events such as an instance needing to control a viewport or another non thread safe request
struct UpstreamOrbitViewport2DEvent {
	Ogre::Vector2f RelativeMotion;
	GameInstance* InstanceRequesting;

	UpstreamOrbitViewport2DEvent(Ogre::Vector2f RM, GameInstance* IR) 
		: RelativeMotion(RM)
		, InstanceRequesting(IR){}
};