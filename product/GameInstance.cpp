// Copyright (c) 2025 Henry Frodsham
#include "GameInstance.h"

#include "InstanceEvent.h"

// a game instance class, a self contained class responsible for coordinating
// and running an instance thread
GameInstance::GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ,
                           InputDevice* Device,
                           InputTranslator* DeviceTranslator,
                           InteractionWheel* NewUIInteractionWheel,
                           PlayerUI* PlayerInterface, Player* InstancePlayer,
                           int ThreadNumber)
    : ParentReporter(ParentR),
      UpstreamQueue(ParentQ),
      InstanceDevice(Device),
      InstanceTranslator(DeviceTranslator),
      InstanceNumber(ThreadNumber),
      PlayerUIOverlay(PlayerInterface),
      InstanceUIWheel(NewUIInteractionWheel),
      GamePlayer(InstancePlayer) {
  LocalBus = new EventBus();
  LocalQueue = new EventQueue(LocalBus);

  LocalBus->Subscribe<ResizedViewPortEvent>(
      std::bind(&InputTranslator::ResizeViewPortDimensions, InstanceTranslator,
                std::placeholders::_1));

  InstanceCursor = new Cursor(LocalBus, InstanceTranslator, InstanceNumber);

  PlayerControl = new PlayerGeneralControl(InstanceTranslator, UpstreamQueue,
                                           InstanceUIWheel);
  SetUpResponsibility();
}

void GameInstance::Run(float DT) {
  InstanceTranslator->Update(DT);
  InstanceUIWheel->UpdateAndWarmupContext();
  PlayerUIOverlay->Update();
  PlayerControl->Update(DT);
  LocalQueue->Dispatch();
  GamePlayer->Update();
}

void GameInstance::SetUpResponsibility() {
  // lambda required here because GetRelativeMotion needs to be updated
  // dynamically
  PlayerControl->TriggerBus->Subscribe<CameraControlTrigger>(
      [this](CameraControlTrigger trigger) {
        UpstreamQueue->Enqueue(UpstreamOrbitViewport2DEvent(
            InstanceTranslator->GetRelativeMotion(), this));
      });
  PlayerControl->TriggerBus->Subscribe<MouseWheelTrigger>(
      [this](MouseWheelTrigger trigger) {
        UpstreamQueue->Enqueue(ChangeOrbitDepthEvent(
            InstanceTranslator->GetMouseWheelY(), this));
      });
  PlayerControl->TriggerBus->Subscribe<RelativeMotionTrigger>(
      std::bind(&Cursor::Update, InstanceCursor));
  PlayerControl->TriggerBus->Subscribe<RelativeMotionTrigger>(
      std::bind(&InteractionWheel::UpdateAndWarmupContext, InstanceUIWheel));
}
