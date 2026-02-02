// Copyright © 2025 Henry Frodsham
#pragma once
#include <thread>
#include <unordered_map>
#include <vector>

#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GameInstance.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "InstanceEvent.h"
#include "RenderSystem.h"
#include "ViewPortController.h"
#include "ResizeEvent.h"

// the class that "holds the reigns" over all instances

class InstanceOverseer {
 public:
  InstanceOverseer(InputListener* ParentListener);
  EventQueue* InstanceQueue;
  void ReviseAndUpdate(float DeltaTime);

 private:
  std::vector<GameInstance*> GameInstances;

  // viewports need to be stored
  std::unordered_map<GameInstance*, ViewPortController*> InstanceViewports;
  std::unordered_map<GameInstance*, std::thread*> ReusableThreads;

  EventBus* InstanceBus;

  ErrorReporter* InstanceReporter;

  void RegisterNewInstance(RegisterInstanceEvent Event);

  void RecalculateViewPortSizes(RecheckViewPortSizeCommand Cmd);

  // non thread safe requests that must be handled here
  void MoveViewport2DOrbit(UpstreamOrbitViewport2DEvent Event);

  // a new instance is made solely based on a new device being connected
  InputListener* DeviceListener;

};
