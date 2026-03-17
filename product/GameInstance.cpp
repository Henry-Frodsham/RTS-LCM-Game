#include "GameInstance.h"

#include "InstanceEvent.h"

GameInstance::GameInstance(ErrorReporter* ParentR, EventQueue* ParentQ,
                           InputDevice* Device,
                           InputTranslator* DeviceTranslator,
                           InteractionWheel* NewUIInteractionWheel,
                           int ThreadNumber)
    : ParentReporter(ParentR),
      UpstreamQueue(ParentQ),
      InstanceDevice(Device),
      InstanceTranslator(DeviceTranslator),
      InstanceNumber(ThreadNumber),
      PlayerForInstance(new Player(new Empire())),
      InstanceUIWheel(NewUIInteractionWheel) {
  LocalBus = new EventBus();
  LocalQueue = new EventQueue(LocalBus);

  LocalBus->Subscribe<ResizedViewPortEvent>(
      std::bind(&InputTranslator::ResizeViewPortDimensions, InstanceTranslator,
                std::placeholders::_1));

  InstanceCursor = new Cursor(LocalBus, InstanceTranslator, InstanceNumber);

  PlayerControl = new PlayerGeneralControl(InstanceTranslator, UpstreamQueue);
  SetUpResponsibility();
}

void GameInstance::Run(float DT) {
  InstanceTranslator->Update(DT);

  PlayerControl->Update(DT);
  LocalQueue->Dispatch();
}

void GameInstance::SetUpResponsibility() {
  // lambda required here because GetRelativeMotion needs to be updated
  // dynamically
  PlayerControl->TriggerBus->Subscribe<CameraControlTrigger>(
      [this](CameraControlTrigger trigger) {
        UpstreamQueue->Enqueue(UpstreamOrbitViewport2DEvent(
            InstanceTranslator->GetRelativeMotion(), this));
      });

  PlayerControl->TriggerBus->Subscribe<RelativeMotionTrigger>(
      std::bind(&Cursor::Update, InstanceCursor));
  PlayerControl->TriggerBus->Subscribe<RelativeMotionTrigger>(
      std::bind(&InteractionWheel::UpdateAndWarmupContext, InstanceUIWheel));
}
