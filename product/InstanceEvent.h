// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include "GameInstance.h"
#include "InputDevice.h"

struct RegisterInstanceEvent {
  InputDevice* InstanceDevice;
};

// for events such as an instance needing to control a viewport or another non
// thread safe request
struct UpstreamOrbitViewport2DEvent {
  Ogre::Vector2f RelativeMotion;
  GameInstance* InstanceRequesting;

  UpstreamOrbitViewport2DEvent(Ogre::Vector2f RM, GameInstance* IR)
      : RelativeMotion(RM), InstanceRequesting(IR) {}
};

struct ChangeOrbitDepthEvent {
  int MouseWheelY;
  GameInstance* InstanceRequesting;
  ChangeOrbitDepthEvent(int WheelY, GameInstance* Instance) : MouseWheelY(WheelY), InstanceRequesting(Instance) {}
};
struct RecheckViewPortSizeCommand {};
