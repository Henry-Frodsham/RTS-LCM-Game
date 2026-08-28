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

  PlayerTranslator->ActionBus->Subscribe<DragSelectCommand>(std::bind(
      &PlayerGeneralControl::OnSelectDrag, this, std::placeholders::_1));
  TriggerBus->Subscribe<RayPickResult>(std::bind(
      &PlayerGeneralControl::OnCompletedTrace, this, std::placeholders::_1));
  TriggerBus->Subscribe<BoxPickResult>(std::bind(
      &PlayerGeneralControl::OnCompletedBoxSelect, this,
      std::placeholders::_1));

  SelectionRectangle = new SelectionBox(
      "selection_box", PlayerTranslator->ManagedDevice, "GREEN",
      PlayerTranslator->GetThreadNumber());
}

void PlayerGeneralControl::Update(float Dt) {
  // while a unit is selected, holding the orbit modifier (right mouse or
  // left trigger) drives the hold-to-preview move gesture instead of
  // orbiting the camera - orbiting only applies with nothing selected
  const bool Selected = InteractionWheelToNotify->HasSelection();
  const bool Holding = PlayerTranslator->HoldingRMBorLT();
  const bool Previewing = Selected && Holding;

  if (Previewing) {
    TriggerPreviewRayTrace();
  } else if (PlayerTranslator->HasRelativeMotion() && Holding) {
    TriggerQueue->Enqueue(CameraControlTrigger());
  }

  if (WasPreviewing && !Previewing) {
    // modifier released after a preview - commit whatever was last shown
    InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
        CommitPathPreviewEvent());
  }
  InteractionWheelToNotify->SetPreviewActive(Previewing);
  WasPreviewing = Previewing;

  if (PlayerTranslator->HasRelativeMotion()) {
    TriggerQueue->Enqueue(RelativeMotionTrigger());
  }
  if (PlayerTranslator->GetPreciseMouseWheelY() != 0.f) {
    TriggerQueue->Enqueue(MouseWheelTrigger());
  }
  TriggerQueue->Dispatch();
}

// the one place a select gesture becomes a world query. a gesture that never
// travelled far enough to be a box picks with a ray at the point it ended,
// exactly as a click always did; one that did picks with the rectangle it
// swept. either way the world is only asked once, on the release - the press
// edge just opens the gesture
void PlayerGeneralControl::OnSelectDrag(DragSelectCommand Cmd) {
  switch (Cmd.Phase) {
    case DragPhase::Begin:
      // nothing on screen yet. a press that turns out to be a click should
      // never flash a zero sized rubber band
      break;

    case DragPhase::Update:
      // only published once the gesture is a box (see
      // InputTranslator::UpdateSelectDrag), so reaching here is the band
      // earning its place on screen
      SelectionRectangle->SetCorners(Cmd.OriginX, Cmd.OriginY,
                                     Cmd.Context.MouseX, Cmd.Context.MouseY);
      SelectionRectangle->ChangeVisibility(true);
      break;

    case DragPhase::End:
      SelectionRectangle->ChangeVisibility(false);
      if (Cmd.ExceededThreshold) {
        TriggerBoxSelect(Cmd);
      } else {
        TriggerSelectRayTrace(Cmd.Context, Cmd.Additive);
      }
      break;

    case DragPhase::Cancel:
      // the gesture was taken away rather than completed, so the band comes
      // off screen and nothing is picked - the selection is left exactly as
      // the player last left it
      SelectionRectangle->ChangeVisibility(false);
      break;
  }
}

void PlayerGeneralControl::TriggerSelectRayTrace(const ActionContext& Context,
                                                 bool Additive) {
  RenderSystem& RS = RenderSystem::GetInstance();
  std::vector<float> Position =
      std::vector<float>{Context.MouseX, Context.MouseY};
  RS.RenderQueue->Enqueue(StartRayTraceEvent(
      Position, PlayerTranslator->ManagedDevice,
      [](EventQueue* queue, RayPickResult Event) { queue->Enqueue(Event); },
      TriggerQueue, Additive));
}

void PlayerGeneralControl::TriggerBoxSelect(const DragSelectCommand& Cmd) {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(StartBoxSelectEvent(
      std::vector<float>{Cmd.OriginX, Cmd.OriginY},
      std::vector<float>{Cmd.Context.MouseX, Cmd.Context.MouseY},
      PlayerTranslator->ManagedDevice, Cmd.Additive,
      [](EventQueue* queue, BoxPickResult Event) { queue->Enqueue(Event); },
      TriggerQueue));
}

void PlayerGeneralControl::TriggerPreviewRayTrace() {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(StartRayTraceEvent(
      PlayerTranslator->GetNormalizedCursorPosition(),
      PlayerTranslator->ManagedDevice,
      [](EventQueue* queue, RayPickResult Event) { queue->Enqueue(Event); },
      TriggerQueue));
}

// callback function from a completed raytrace in render system
void PlayerGeneralControl::OnCompletedTrace(RayPickResult Event) {
  if (!Event.HasEntity() && Event.Terrain.DidHit) {
    InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
        NotifyRayResult(Event.Terrain.HitPoint, Event.Terrain.SurfaceNormal,
                        Event.Terrain.HitBiome, Event.Additive));
  } else if (Event.HasEntity()) {
    InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
        NotifyEntityResult(Event.HitEntity, Event.Additive));
  }
}

// callback function from a completed box select in render system. an empty
// box is still forwarded - a drag over open ground that selects nothing is
// how a player clears their selection, so it has to reach the wheel rather
// than being dropped here
void PlayerGeneralControl::OnCompletedBoxSelect(BoxPickResult Event) {
  InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
      NotifyBoxSelectResult(Event.Entities, Event.Additive));
}
