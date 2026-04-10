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
  LastDeltaLatLon = Ogre::Vector2f();
}

void PlayerGeneralControl::Update(float Dt) {
  if (PlayerTranslator->HasRelativeMotion() &&
      PlayerTranslator->HoldingRMBorLT()) {
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
      [](EventQueue* queue, EndRayTraceResultEvent Event) {
        queue->Enqueue(Event);
      },
      TriggerQueue));
}

// callback function from a completed raytrace in render system
void PlayerGeneralControl::OnCompletedTrace(EndRayTraceResultEvent Event) {
  RenderSystem& RS = RenderSystem::GetInstance();
  if (Event.RayResult.size() != 0) {
    for (auto Node : Event.RayResult) {
      if (Node.movable->getParentSceneNode()->getName() != "GlobeNode") {
        // downcast because we dont need Ogre::Movable
        if (SelectedEntity != static_cast<Ogre::Entity*>(Node.movable) &&
            SelectedEntity != nullptr) {
          RS.RenderQueue->Enqueue(
              ChangeEntMaterialEvent(SelectedEntity, "WHITE"));
        }
        SelectedEntity = static_cast<Ogre::Entity*>(Node.movable);
        RS.RenderQueue->Enqueue(ChangeEntMaterialEvent(SelectedEntity, "RED"));
        InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
            NotifySelectedEntity(SelectedEntity, false));
        return;

      } else if (Node.movable->getName() != "GlobeBase") {
        Ogre::Vector3 HitPoint = Event.Ray.getPoint(Node.distance);
        Ogre::Vector3 SurfaceNormal =
            (HitPoint - Ogre::Vector3(0.5f, 0.f, -5.f)).normalisedCopy();
        Ogre::Vector3 SnappedPos =
            Ogre::Vector3(0.5f, 0.f, -5.f) + SurfaceNormal * 1.f;

        if (SelectedEntity != nullptr) {
          InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
              NotifyLatLonEvent(LastDeltaLatLon));
        }
        InteractionWheelToNotify->ForeignNotifQueue->Enqueue(
            NotifyPosEvent(SnappedPos));
      }
    }
  }
}

Ogre::Vector2f PlayerGeneralControl::HitPointToDeltaLatLon(
    Ogre::Vector3 UnitPos, Ogre::Vector3 HitPoint) {
  Ogre::Vector3 From = UnitPos.normalisedCopy();
  Ogre::Vector3 To = HitPoint.normalisedCopy();

  float DeltaLon = std::atan2(To.x, To.z) - std::atan2(From.x, From.z);
  float DeltaLat = std::asin(To.y) - std::asin(From.y);

  return Ogre::Vector2f(DeltaLat, DeltaLon);
}
