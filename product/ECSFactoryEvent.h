// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <Ogre/Ogre.h>

#include <entt/entt.hpp>
#include <memory>  // NOLINT(build/include_order)
#include <string>  // NOLINT(build/include_order)

#include "Player.h"
// an event solely used to create a new registry entry
// since an entity could exist without an ogre component its neccessary that is
// standalone example usage: SomeQueue.enqueue(CreateEntityEvent(ERef, "myNode",
// Ogre::Vector3(0.f,0.f,0.f)) then
// -> SomeHandler(CreateEntityEvent Event)...
struct CreateEntityEvent {
  // reference counted ownership of CreatedEntity
  // this event and any local variables storing this entity will be destroyed
  // so its neccessary that it persists in the heap until subsequent events
  // (storing this shared ptr) are also destroyed
  std::shared_ptr<entt::entity> CreatedEntity;

  explicit CreateEntityEvent(std::shared_ptr<entt::entity> ent)
      : CreatedEntity(ent) {}
};

// an event used to add an ogre component to a prexisting entity
// example usage:
// SomeQueue.enqueue(AddOgreComponentEvent(ERef, "myNode",
// Ogre::Vector3(0.f,0.f,0.f)) then
// -> SomeHandler(AddOgreComponentEvent Event)
struct AddOgreComponentEvent {
  std::shared_ptr<entt::entity> Entity;
  std::string NodeName;
  Ogre::Vector3 InitialPosition;
  AddOgreComponentEvent(std::shared_ptr<entt::entity> ent, std::string NodeN,
                        Ogre::Vector3 InitP)
      : Entity(ent), NodeName(NodeN), InitialPosition(InitP) {}
};

// an event used to add a mesh component to a preexisting entity
//  example usage:
//  SomeQueue.enqueue(AddMeshComponentEvent(ERef, "myNode",
//  Ogre::Vector3(0.f,0.f,0.f)) then
//  -> SomeHandler(AddMeshComponentEvent Event)
struct AddMeshComponentEvent {
  std::shared_ptr<entt::entity> Entity;
  std::string MeshName;
  std::string EntityName;

  AddMeshComponentEvent(std::shared_ptr<entt::entity> ent, std::string MeshN,
                        std::string EntN)
      : Entity(ent), MeshName(MeshN), EntityName(EntN) {}
};

// event to add an ownership comp to entity
struct AddOwnerShipComponentEvent {
  std::shared_ptr<entt::entity> Entity;
  int PlayerID;
  Player* GamePlayer;
  AddOwnerShipComponentEvent(std::shared_ptr<entt::entity> Ent, int Id,
                             Player* P)
      : Entity(Ent), PlayerID(Id), GamePlayer(P) {}
};

// event to add a unit comp to entity
struct AddUnitProductionEvent {
  std::shared_ptr<entt::entity> Entity;
  int ProdPerM;
  AddUnitProductionEvent(std::shared_ptr<entt::entity> Ent, int PPM)
      : Entity(Ent), ProdPerM(PPM) {}
};
// event to add a health comp to entity
struct AddHealthEvent {
  std::shared_ptr<entt::entity> Entity;
  float Health;
  AddHealthEvent(std::shared_ptr<entt::entity> Ent, float H)
      : Entity(Ent), Health(H) {}
};
// event to add an attack comp to entity
struct AddAttackEvent {
  std::shared_ptr<entt::entity> Entity;
  float Radius;
  float Damage;
  AddAttackEvent(std::shared_ptr<entt::entity> Ent, float R, float D)
      : Entity(Ent), Radius(R), Damage(D) {}
};
// event to change an entities visibility, foreign to render system
struct ChangeEntityVisibilityEvent {
  bool Visible;
  std::string NodeName;
  ChangeEntityVisibilityEvent(std::string ent, bool Vis)
      : NodeName(ent), Visible(Vis) {}
};

// event to move an entity around a spherical origin
struct MoveEntityAlongSphericalEvent {
  Ogre::Entity* Unit;
  Ogre::Real Radius;
  Ogre::Vector3f TargetPos;
  MoveEntityAlongSphericalEvent(Ogre::Entity* U, Ogre::Real R,
                                Ogre::Vector3f TP)
      : Unit(U), Radius(R), TargetPos(TP) {}
};

// orientate an entity to surface normal event, so it sits flat
struct OrientateEntityEvent {
  std::shared_ptr<entt::entity> Entity;
  Ogre::Vector3f SurfaceNormal;
  explicit OrientateEntityEvent(
      std::shared_ptr<entt::entity> Ent,
      Ogre::Vector3f Sn)  // NOLINT(whitespace/line_length)
      : Entity(Ent), SurfaceNormal(Sn) {}
};

struct AddRangeComponentEvent {
  std::shared_ptr<entt::entity> Entity;
  AddRangeComponentEvent(std::shared_ptr<entt::entity> Ent)
      : Entity(Ent) {}
};

struct NotifyConsequentialEntityStateChange {
  Ogre::Entity* Entity;
  NotifyConsequentialEntityStateChange(Ogre::Entity* Ent) : Entity(Ent) {}
};