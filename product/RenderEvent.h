// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <entt/entt.hpp>

#include "CommonGameObjectComponents.h"

// only used here as a pointer type - previously relied on whichever
// translation unit happened to #include "ViewPortController.h" before this
// file (e.g. via OverlayEvent.h), which broke as soon as a header pulling
// in RenderEvent.h first was added ahead of that in RenderSystem.h
class ViewPortController;

// common events issued to render system
// self explanatory
// e.g ScaleEntityEvent - Scales an entity

struct CreateSceneNodeEvent {
  std::reference_wrapper<Ogre::SceneNode*> Node;
  std::string NodeName;
  CreateSceneNodeEvent(Ogre::SceneNode*& N, std::string NodeN)
      : Node(N), NodeName(NodeN) {}
};

struct CreateOgreEntityEvent {
  std::reference_wrapper<Ogre::Entity*> Entity;
  std::string EntityName;
  std::string MeshName;
  entt::entity OwningEntity;
  Ogre::uint32 QueryFlags;
  CreateOgreEntityEvent(Ogre::Entity*& Ent, std::string EN, std::string MN,
                        entt::entity Entt, Ogre::uint32 Flags)
      : Entity(Ent), EntityName(EN), MeshName(MN), OwningEntity(Entt), QueryFlags(Flags) {}
};

struct SetNodePositionEvent {
  std::reference_wrapper<Ogre::SceneNode*> NodeToChange;
  Ogre::Vector3 NewPosition;
  SetNodePositionEvent(Ogre::SceneNode*& N, Ogre::Vector3 Pos)
      : NodeToChange(N), NewPosition(Pos) {}
};

struct AttachEntityToScenNodeEvent {
  std::reference_wrapper<Ogre::Entity*> Entity;
  std::reference_wrapper<Ogre::SceneNode*> SceneNode;
  AttachEntityToScenNodeEvent(Ogre::Entity*& Ent, Ogre::SceneNode*& N)
      : Entity(Ent), SceneNode(N) {}
};

struct ScaleEntityEvent {
  std::string EntName;
  float NewScale;
  ScaleEntityEvent(std::string EntN, float NewS)
      : EntName(EntN), NewScale(NewS) {}
};

struct SetEntPositionEvent {
  Ogre::Entity* Ent;
  Ogre::Vector3f Vec;
  SetEntPositionEvent(Ogre::Entity* N, Ogre::Vector3f P) : Ent(N), Vec(P) {}
};

struct RotateEntToSurfaceNormalEvent {
  Ogre::Entity* Entity;
  Ogre::Vector3f SurfaceNormal;  // was RelativeRotCentre
  RotateEntToSurfaceNormalEvent(Ogre::Entity* Ent, Ogre::Vector3f Normal)
      : Entity(Ent), SurfaceNormal(Normal) {}
};

struct ChangeEntMaterialEvent {
  Ogre::Entity* Ent;
  std::string MatName;
  ChangeEntMaterialEvent(Ogre::Entity* Entity, std::string Material)
      : Ent(Entity), MatName(Material) {}
};

struct AddOwnerShipToEntEvent {
  Ogre::SceneNode* Node;
  int OwnershipId;
  AddOwnerShipToEntEvent(Ogre::SceneNode* N, int Id)
      : Node(N), OwnershipId(Id) {}
};

// creates a unit's health bar (background + fill billboard), parented to
// its scene node so it inherits position/orientation for free. handled by
// HealthBarController, which builds it out of BillboardController's
// generic billboard primitives. OutSet/OutFill mirror the
// reference_wrapper out-param convention used by CreateSceneNodeEvent etc.
// UnitEntity is the unit's own mesh - HealthBarController measures its
// bounding box to anchor the bar above the model instead of guessing at an
// offset (see HealthBarController::CreateHealthBar for why)
struct CreateHealthBarEvent {
  std::reference_wrapper<Ogre::BillboardSet*> OutSet;
  std::reference_wrapper<Ogre::Billboard*> OutFill;
  Ogre::SceneNode* ParentNode;
  Ogre::Entity* UnitEntity;
  CreateHealthBarEvent(Ogre::BillboardSet*& OutS, Ogre::Billboard*& OutF,
                       Ogre::SceneNode* Parent, Ogre::Entity* UnitEnt)
      : OutSet(OutS), OutFill(OutF), ParentNode(Parent), UnitEntity(UnitEnt) {}
};

// resizes a unit's health bar fill billboard to Ratio (0-1) of max health.
// pushed from WorldManager's combat loop, throttled to ~1/10th-of-max-health
// steps rather than every damage tick - see HealthBarComponent
struct UpdateHealthBarEvent {
  Ogre::Billboard* Fill;
  float Ratio;
  UpdateHealthBarEvent(Ogre::Billboard* F, float R) : Fill(F), Ratio(R) {}
};

// creates a unit's facing-direction arrow: a small flat triangle, built once
// in the arrow node's own local space (tip along local +Z) and parented to
// the unit's node so it inherits position for free. Handled directly by
// RenderSystem, same as the path preview ribbon, since like that ribbon it's
// a ManualObject rather than a billboard. WorldForward/WorldUp give the
// arrow's initial world orientation - see RenderSystem::ComputeFacingOrientation
// for why this can't just inherit the parent node's orientation. Starts
// hidden - only shown while the unit is selected (see
// ChangeFacingArrowVisibilityEvent)
struct CreateFacingArrowEvent {
  std::reference_wrapper<Ogre::SceneNode*> OutNode;
  std::reference_wrapper<Ogre::ManualObject*> OutObject;
  Ogre::SceneNode* ParentNode;
  Ogre::Entity* UnitEntity;
  int OwnerID;
  Ogre::Vector3f WorldForward;
  Ogre::Vector3f WorldUp;
  CreateFacingArrowEvent(Ogre::SceneNode*& OutN, Ogre::ManualObject*& OutO,
                         Ogre::SceneNode* Parent, Ogre::Entity* UnitEnt,
                         int Owner, Ogre::Vector3f Forward, Ogre::Vector3f Up)
      : OutNode(OutN),
        OutObject(OutO),
        ParentNode(Parent),
        UnitEntity(UnitEnt),
        OwnerID(Owner),
        WorldForward(Forward),
        WorldUp(Up) {}
};

// re-orients a unit's facing arrow to point along WorldForward, staying
// flush against the tangent plane of WorldUp (the surface normal). doesn't
// touch geometry - only the node's orientation, which is all that ever needs
// to change as a unit turns
struct UpdateFacingArrowOrientationEvent {
  Ogre::SceneNode* ArrowNode;
  Ogre::Vector3f WorldForward;
  Ogre::Vector3f WorldUp;
  UpdateFacingArrowOrientationEvent(Ogre::SceneNode* Node,
                                    Ogre::Vector3f Forward, Ogre::Vector3f Up)
      : ArrowNode(Node), WorldForward(Forward), WorldUp(Up) {}
};

struct ChangeFacingArrowVisibilityEvent {
  Ogre::SceneNode* ArrowNode;
  bool Visible;
  ChangeFacingArrowVisibilityEvent(Ogre::SceneNode* Node, bool Vis)
      : ArrowNode(Node), Visible(Vis) {}
};

struct DestroyNodeEvent {
  Ogre::SceneNode* NodeToDestroy;
  explicit DestroyNodeEvent(Ogre::SceneNode* Node) : NodeToDestroy(Node) {}
};

struct DestroyEntityEvent {
  Ogre::Entity* EntityToDestroy;
  explicit DestroyEntityEvent(Ogre::Entity* Node) : EntityToDestroy(Node) {}
};

struct RevalEntityRangeEvent {
  Ogre::Entity* EntToCheck;
  EventQueue* CallBackQueue;
  float GeneralRange;
  RevalEntityRangeEvent(Ogre::Entity* Ent, EventQueue* CBQ, float GR)
      : EntToCheck(Ent), CallBackQueue(CBQ), GeneralRange(GR) {}
};

struct ChangeCameraOrbitAngleEvent {
  ViewPortController* ViewportToControl;
  Ogre::Vector2f RelativeMotion;
  ChangeCameraOrbitAngleEvent(Ogre::Vector2f RelMotion, ViewPortController* VPC)
      : RelativeMotion(RelMotion), ViewportToControl(VPC) {}
};

struct ChangeCameraOrbitDepthEvent {
  ViewPortController* ViewportToControl;
  float WheelDelta;
  ChangeCameraOrbitDepthEvent(float WheelY, ViewPortController* VPC)
      : WheelDelta(WheelY), ViewportToControl(VPC) {}
};

// draws (or hides) the hold-to-preview path line. Points are world-space,
// in walk order starting from the previewing unit's current position.
// OwnerID is the previewing player's id - PATH_PREVIEW.material reads it
// against viewingPlayerID to discard the line on every other viewport, same
// mechanism as RED_UNIT/WHITE (see RenderSystem::AddOwnerShipToEnt).
// unused when Visible is false, so hide calls can omit it
struct UpdatePathPreviewEvent {
  std::vector<Ogre::Vector3f> Points;
  bool Visible;
  int OwnerID;
  UpdatePathPreviewEvent(std::vector<Ogre::Vector3f> Pts, bool Vis,
                         int Owner = -1)
      : Points(std::move(Pts)), Visible(Vis), OwnerID(Owner) {}
};
