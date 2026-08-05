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
  TriggerBus->Subscribe<RayPickResult>(std::bind(
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
  if (PlayerTranslator->GetPreciseMouseWheelY() != 0.f) {
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
      [](EventQueue* queue, RayPickResult Event) { queue->Enqueue(Event); },
      TriggerQueue));
}

// callback function from a completed raytrace in render system
void PlayerGeneralControl::OnCompletedTrace(RayPickResult Event) {
  if (!Event.HasEntity() && Event.Terrain.DidHit) {
    InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
        NotifyRayResult(Event.Terrain.HitPoint, Event.Terrain.SurfaceNormal,
                        Event.Terrain.HitBiome));
  } else if (Event.HasEntity()) {
    InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
        NotifyEntityResult(Event.HitEntity));
  }
}
