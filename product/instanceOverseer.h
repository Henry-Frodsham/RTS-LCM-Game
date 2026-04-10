// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <BS_thread_pool.hpp>
#include <thread>  // NOLINT(build/include_order)
#include <unordered_map>  // NOLINT(build/include_order)
#include <vector>  // NOLINT(build/include_order)

#include "ECSHelper.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GameInstance.h"
#include "InputListener.h"
#include "InputTranslator.h"
#include "InstanceEvent.h"
#include "InteractionWheel.h"
#include "PlayerUI.h"
#include "RenderSystem.h"
#include "ResizeEvent.h"
#include "ViewPortController.h"

// the class that "holds the reigns" over all instances

class InstanceOverseer {
 public:
  InstanceOverseer(InputListener* ParentListener, ECSHelper* Interactor);
  EventQueue* InstanceQueue;
  void ReviseAndUpdate(float DeltaTime);

 private:
  std::vector<GameInstance*> GameInstances;

  // viewports need to be stored
  std::unordered_map<GameInstance*, ViewPortController*> InstanceViewports;
  BS::thread_pool InstanceThreadPool;

  EventBus* InstanceBus;
  ECSHelper* Factory;
  ErrorReporter* InstanceReporter;

  void RegisterNewInstance(RegisterInstanceEvent Event);

  void RecalculateViewPortSizes(RecheckViewPortSizeCommand Cmd);

  // non thread safe requests that must be handled here
  void MoveViewport2DOrbit(UpstreamOrbitViewport2DEvent Event);

  // a new instance is made solely based on a new device being connected
  InputListener* DeviceListener;
};
