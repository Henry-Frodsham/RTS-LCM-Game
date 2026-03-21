#include "PlayerGeneralControl.h"

PlayerGeneralControl::PlayerGeneralControl(InputTranslator* Translator,
                                           EventQueue* Queue)
    : PlayerTranslator(Translator), ControlQueue(Queue) {
  TriggerBus = new EventBus();
  TriggerQueue = new EventQueue(TriggerBus);

  PlayerTranslator->ActionBus->Subscribe<PressActionCommand>(
      std::bind(&PlayerGeneralControl::OnPress, this, std::placeholders::_1));
  PlayerTranslator->ActionBus->Subscribe<EndRayTraceResultEvent>(std::bind(
      &PlayerGeneralControl::OnCompletedTrace, this, std::placeholders::_1));
}

void PlayerGeneralControl::Update(float Dt) {
  if (PlayerTranslator->HasRelativeMotion() &&
      PlayerTranslator->HoldingRMBorRT()) {
    TriggerQueue->Enqueue(CameraControlTrigger());
  }
  if (PlayerTranslator->HasRelativeMotion()) {
    TriggerQueue->Enqueue(RelativeMotionTrigger());
  }
  TriggerQueue->Dispatch();
}

void PlayerGeneralControl::OnPress(PressActionCommand Cmd) {
  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> Position =
      std::vector<float>{Cmd.Context.MouseX, Cmd.Context.MouseY};
  RS.RenderQueue->Enqueue(StartRayTraceEvent(
      Position, PlayerTranslator->ManagedDevice,
      [](EventQueue* queue,EndRayTraceResultEvent Event) {
        queue->Enqueue(Event);
      },
      TriggerQueue));
}

void PlayerGeneralControl::OnCompletedTrace(EndRayTraceResultEvent Event) {
  if (Event.RayResult.size() != 0) {
    int x = 0;
  }
}