// Copyright (c) 2025 Henry Frodsham
#include "instanceOverseer.h"

#include <vector>
InstanceOverseer::InstanceOverseer(InputListener* ParentListener,  //NOLINT [whitespace/line_length]
                                   ECSHelper* Interactor)
    : DeviceListener(ParentListener),
      InstanceThreadPool(std::thread::hardware_concurrency()),
      Factory(Interactor) {
  InstanceBus = new EventBus();
  InstanceQueue = new EventQueue(InstanceBus);
  InstanceReporter = new ErrorReporter();
  InstanceBus->Subscribe<RegisterInstanceEvent>(std::bind(
      &InstanceOverseer::RegisterNewInstance, this, std::placeholders::_1));
  InstanceBus->Subscribe<UpstreamOrbitViewport2DEvent>(std::bind(
      &InstanceOverseer::MoveViewport2DOrbit, this, std::placeholders::_1));
  InstanceBus->Subscribe<ChangeOrbitDepthEvent>(std::bind(
      &InstanceOverseer::ChangeOrbitDepth, this, std::placeholders::_1));
  InstanceBus->Subscribe<RecheckViewPortSizeCommand>(
      std::bind(&InstanceOverseer::RecalculateViewPortSizes, this,
                std::placeholders::_1));
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
  } else {
    VP = RS.CreateViewPort();
  }
  std::vector<float> RelativeVPDimensions = VP->GetViewPortDimensions();
  Ogre::RenderWindowDescription WindowInfo = RS.GetPrimaryWindowInformation();
  float TotalWindowWidth = static_cast<float>(WindowInfo.width);
  float TotalWindowHeight = static_cast<float>(WindowInfo.height);

  float ViewPortWidth = TotalWindowWidth * RelativeVPDimensions[2];
  float ViewPortHeight = TotalWindowWidth * RelativeVPDimensions[3];

  InputTranslator* Translator = new InputTranslator(
      Event.InstanceDevice, ViewPortWidth, ViewPortHeight,
      GameInstances.size() + 1, RS.GetRenderWindowDimensions());
  DeviceListener->AddListenerQueue(Event.InstanceDevice,
                                   Translator->WaitingEvents);
  Player* InstancePlayer = new Player(GameInstances.size() + 1);
  InteractionWheel* NewUIInteractionWheel = new InteractionWheel(
      Translator, GameInstances.size() + 1, Factory, InstancePlayer);
  PlayerUI* NewPlayerUI =
      new PlayerUI(InstancePlayer, GameInstances.size() + 1);
  GameInstance* NewInstance =
      new GameInstance(InstanceReporter, InstanceQueue, Event.InstanceDevice,
                       Translator, NewUIInteractionWheel, NewPlayerUI,
                       InstancePlayer, GameInstances.size() + 1);

  GameInstances.push_back(NewInstance);
  InstanceViewports.emplace(NewInstance, VP);

  VP->RegisterControllingDevice(Event.InstanceDevice);

  InstanceQueue->Enqueue(RecheckViewPortSizeCommand());
}

void InstanceOverseer::RecalculateViewPortSizes(
    RecheckViewPortSizeCommand Cmd) {
  for (auto Pair : InstanceViewports) {
    std::vector<int> Actual = Pair.second->GetActualDimensions();
    Pair.first->LocalQueue->Enqueue(ResizedViewPortEvent(
        static_cast<float>(Actual[0]), static_cast<float>(Actual[1])));
  }
}

void InstanceOverseer::ReviseAndUpdate(float DeltaTime) {
  std::vector<InputDevice*> NewInstanceDevices =
      DeviceListener->GetUnintegratedDevices();
  if (NewInstanceDevices.size() > 0) {
    for (InputDevice* D : NewInstanceDevices) {
      InstanceQueue->Enqueue(RegisterInstanceEvent(D));
    }
  }

  std::vector<std::future<void>> Futures;
  Futures.reserve(GameInstances.size());

  for (GameInstance* Instance : GameInstances) {
    Futures.push_back(InstanceThreadPool.submit_task(
        [Instance, DeltaTime]() { Instance->Run(DeltaTime); }));
  }

  for (auto& Future : Futures) {
    Future.wait();
  }

  InstanceQueue->Dispatch();
  InstanceReporter->Dispatch();
}

void InstanceOverseer::MoveViewport2DOrbit(UpstreamOrbitViewport2DEvent Event) {
  RenderSystem& RS = RenderSystem::GetInstance();
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

  RS.RenderQueue->Enqueue(
      ChangeCameraOrbitAngleEvent(Event.RelativeMotion, ViewPortToMove));
}

void InstanceOverseer::ChangeOrbitDepth(ChangeOrbitDepthEvent Event) {
  RenderSystem& RS = RenderSystem::GetInstance();
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

  RS.RenderQueue->Enqueue(
      ChangeCameraOrbitDepthEvent(Event.WheelDelta, ViewPortToMove));
}
