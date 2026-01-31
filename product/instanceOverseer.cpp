// Copyright © 2025 Henry Frodsham
#include "instanceOverseer.h"

InstanceOverseer::InstanceOverseer(InputListener* ParentListener)
    : DeviceListener(ParentListener) {
  InstanceBus = new EventBus();
  InstanceQueue = new EventQueue(InstanceBus);
  InstanceReporter = new ErrorReporter();
  InstanceBus->Subscribe<RegisterInstanceEvent>(std::bind(
      &InstanceOverseer::RegisterNewInstance, this, std::placeholders::_1));
  InstanceBus->Subscribe<UpstreamOrbitViewport2DEvent>(std::bind(
      &InstanceOverseer::MoveViewport2DOrbit, this, std::placeholders::_1));
  InstanceBus->Subscribe<RecheckViewPortSizeCommand>(std::bind(
      &InstanceOverseer::RecalculateViewPortSizes, this, std::placeholders::_1));
}

void InstanceOverseer::RegisterNewInstance(RegisterInstanceEvent Event) {
  if (!Event.InstanceDevice) {
    InstanceReporter->EnqueueError(
        ErrorDetail::CreateError(ErrorCode::INSTANCE_REQUEST_NO_DEVICE));
    return;
  }
  RenderSystem& RS = RenderSystem::GetInstance();

  ViewPortController* VP = nullptr;
  // KBM should always have control of the main viewport for text prompts etc
  // if the new device isnt the kbm then just make a new one
  if (Event.InstanceDevice->InputType == InputDeviceType::KBM) {
      VP = RS.GetPrimaryViewport();
  }
  else {
      VP = RS.CreateViewPort();
  }
  std::vector<float> RelativeVPDimensions = VP->GetViewPortDimensions();
  Ogre::RenderWindowDescription WindowInfo =
      RS.GetPrimaryWindowInformation();
  float TotalWindowWidth = static_cast<float>(WindowInfo.width);
  float TotalWindowHeight = static_cast<float>(WindowInfo.height);

  float ViewPortWidth = TotalWindowWidth * RelativeVPDimensions[2];
  float ViewPortHeight = TotalWindowWidth * RelativeVPDimensions[3];

  InputTranslator* Translator = new InputTranslator(Event.InstanceDevice, ViewPortWidth, ViewPortHeight);
  DeviceListener->AddListenerQueue(Event.InstanceDevice,
                                   Translator->WaitingEvents);
  GameInstance* NewInstance = new GameInstance(InstanceReporter, InstanceQueue, Event.InstanceDevice,
                       Translator, GameInstances.size() + 1);

  GameInstances.push_back(NewInstance);
  InstanceViewports.emplace(NewInstance, VP);



  VP->RegisterControllingDevice(Event.InstanceDevice);

  InstanceQueue->Enqueue(RecheckViewPortSizeCommand());
}

void InstanceOverseer::RecalculateViewPortSizes(RecheckViewPortSizeCommand Cmd) {
    RenderSystem& RS = RenderSystem::GetInstance();
    Ogre::RenderWindowDescription WindowInfo =
        RS.GetPrimaryWindowInformation();
    for (auto Pair : InstanceViewports) {
        std::vector<float> RelativeVPDimensions = Pair.second->GetViewPortDimensions();

        float TotalWindowWidth = static_cast<float>(WindowInfo.width);
        float TotalWindowHeight = static_cast<float>(WindowInfo.height);

        float ViewPortWidth = TotalWindowWidth * RelativeVPDimensions[2];
        float ViewPortHeight = TotalWindowHeight * RelativeVPDimensions[3];

        Pair.first->LocalQueue->Enqueue(ResizedViewPortEvent(ViewPortWidth, ViewPortHeight));
    }
}

void InstanceOverseer::ReviseAndUpdate(float DeltaTime) {

  std::vector<InputDevice*> NewInstanceDevices =
      DeviceListener->GetUnintegratedDevices();
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
  InstanceQueue->Dispatch();
  InstanceReporter->Dispatch();
}

void InstanceOverseer::MoveViewport2DOrbit(UpstreamOrbitViewport2DEvent Event) {
  ViewPortController* ViewPortToMove = nullptr;
  try {
    ViewPortToMove = InstanceViewports.at(Event.InstanceRequesting);
  } catch (std::exception e) {
    InstanceReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::BAD_VIEWPORT_CONTROL_REQUEST,
        fmt::format("a game instance was not associated with any viewport to "
                    "be controlled")));
    return;
  }
  // temporary demonstration using a fixed orbit point
  // in future this will be set in the event
  ViewPortToMove->MoveCameraOrbitingPoint2DMotion(
      Event.RelativeMotion, Ogre::Vector3f(0.5f, 0.f, -5.f));
}
