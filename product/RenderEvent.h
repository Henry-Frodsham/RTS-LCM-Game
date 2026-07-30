// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "CommonGameObjectComponents.h"

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
  CreateOgreEntityEvent(Ogre::Entity*& Ent, std::string EN, std::string MN)
      : Entity(Ent), EntityName(EN), MeshName(MN) {}
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
  Ogre::Vector3f RelativeRotCentre;
  RotateEntToSurfaceNormalEvent(Ogre::Entity* Ent, Ogre::Vector3f RRC)
      : Entity(Ent), RelativeRotCentre(RRC) {}
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
  int MouseWheelY;
  ChangeCameraOrbitDepthEvent(int WheelY, ViewPortController* VPC)
      : MouseWheelY(WheelY), ViewportToControl(VPC) {}
};
