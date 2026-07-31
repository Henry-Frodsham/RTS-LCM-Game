// Copyright (c) 2025 Henry Frodsham
#include "PlayerGeneralControl.h"

#include <vector>

// general player control, class controlling input translator events and
// transforming them into camera control and interactions
PlayerGeneralControl::PlayerGeneralControl(InputTranslator* Translator,
                                           EventQueue* Queue,
                                           InteractionWheel* Wheel)
    : PlayerTranslator(Translator),
      ControlQueue(Queue),
      InteractionWheelToNotify(Wheel) {
  TriggerBus = new EventBus();
  TriggerQueue = new EventQueue(TriggerBus);

  PlayerTranslator->ActionBus->Subscribe<PressActionCommand>(
      std::bind(&PlayerGeneralControl::OnPress, this, std::placeholders::_1));
  TriggerBus->Subscribe<EndRayTraceResultEvent>(std::bind(
      &PlayerGeneralControl::OnCompletedTrace, this, std::placeholders::_1));

  SelectedEntity = nullptr;
}

void PlayerGeneralControl::Update(float Dt) {
  if (PlayerTranslator->HasRelativeMotion() &&
      PlayerTranslator->HoldingRMBorLT()) {
    TriggerQueue->Enqueue(CameraControlTrigger());
  }
  if (PlayerTranslator->HasRelativeMotion()) {
    TriggerQueue->Enqueue(RelativeMotionTrigger());
  }
  if (PlayerTranslator->GetMouseWheelY() != 0) {
    TriggerQueue->Enqueue(MouseWheelTrigger());
  }
  TriggerQueue->Dispatch();
}

void PlayerGeneralControl::OnPress(PressActionCommand Cmd) {
  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> Position =
      std::vector<float>{Cmd.Context.MouseX, Cmd.Context.MouseY};
  RS.RenderQueue->Enqueue(StartRayTraceEvent(
      Position, PlayerTranslator->ManagedDevice,
      [](EventQueue* queue, EndRayTraceResultEvent Event) {
        queue->Enqueue(Event);
      },
      TriggerQueue));
}

// callback function from a completed raytrace in render system
void PlayerGeneralControl::OnCompletedTrace(EndRayTraceResultEvent Event) {
  if (!Event.DidHit) {
    return;
  }
  InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
      NotifySurfaceNormalEvent(Event.SurfaceNormal));
  InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
      NotifyPosEvent(Event.HitPoint));
}
