#pragma once
#include <Ogre/Ogre.h>

#include <entt/entt.hpp>

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

  CreateEntityEvent(std::shared_ptr<entt::entity> ent) : CreatedEntity(ent) {}
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
