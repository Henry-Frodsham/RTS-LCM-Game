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

struct CacheRangeQueryEvent {
  float GeneralRange = 10.f;
  std::unordered_map<Ogre::SceneNode*, OwnershipComponent> Entities;
  EventQueue* Queue;
  CacheRangeQueryEvent(
      float GR, std::unordered_map<Ogre::SceneNode*, OwnershipComponent> Ents,
      EventQueue* Q)
      : GeneralRange(GR), Entities(Ents), Queue(Q) {}
};

struct DestroyNodeEvent {
  Ogre::SceneNode* NodeToDestroy;
  explicit DestroyNodeEvent(Ogre::SceneNode* Node) : NodeToDestroy(Node) {}
};

struct DestroyEntityEvent {
  Ogre::Entity* EntityToDestroy;
  explicit DestroyEntityEvent(Ogre::Entity* Node) : EntityToDestroy(Node) {}
};
