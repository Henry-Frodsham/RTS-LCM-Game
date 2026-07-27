// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <array>
#include <entt/entt.hpp>
#include <functional>     // NOLINT(build/include_order)
#include <memory>         // NOLINT(build/include_order)
#include <string>         // NOLINT(build/include_order)
#include <unordered_map>  // NOLINT(build/include_order)
#include <vector>         // NOLINT(build/include_order)

#include "ECSFactoryEvent.h"
#include "ECSHelper.h"
#include "WorldEvent.h"

// basic helper functions to construct common entity types easily
namespace EntityTemplates {
// construct a basic entity with an attached sceneNode
inline std::shared_ptr<entt::entity> ConstructBasicEntity(
    ECSHelper* Factory, CreateBasicWorldEntityEvent Event) {
  // garbage collector will destroy this when the function exits
  // hence why the entity is stored as a shared pointer
  auto Entity = std::make_shared<entt::entity>();

  Factory->FactoryQueue->Enqueue<CreateEntityEvent>(CreateEntityEvent(Entity));

  Factory->FactoryQueue->Enqueue<AddOgreComponentEvent>(
      AddOgreComponentEvent(Entity, Event.NodeName, Event.InitialPosition));

  // return the shared pointer for modular construction
  // wont hold a valid value until the factory is dispatched
  return Entity;
}
// construct an entity with an attached mesh (3d model)
inline std::shared_ptr<entt::entity> ConstructMeshEntity(
    ECSHelper* Factory, CreateMeshWorldEntityEvent Event) {
  auto Entity = ConstructBasicEntity(
      Factory,
      CreateBasicWorldEntityEvent(Event.NodeName, Event.InitialPosition));

  Factory->FactoryQueue->Enqueue<AddMeshComponentEvent>(
      AddMeshComponentEvent(Entity, Event.MeshName, Event.EntityName));

  return Entity;
}
// create any game object with ownership. e.g city
inline std::shared_ptr<entt::entity> CreateGameObject(
    ECSHelper* Factory, CreateGameObjectEvent Event) {
  auto Entity = ConstructMeshEntity(
      Factory,
      CreateMeshWorldEntityEvent(Event.NodeName, Event.MeshName,
                                 Event.EntityName, Event.InitialPosition));

  Factory->FactoryQueue->Enqueue<AddOwnerShipComponentEvent>(
      AddOwnerShipComponentEvent(Entity, Event.PlayerId, Event.GamePlayer));

  Factory->FactoryQueue->Enqueue<OrientateEntityEvent>(
      OrientateEntityEvent(Entity));
  return Entity;
}
// template to produce a unit producing entity
inline std::shared_ptr<entt::entity> CreateUnitProducingGameObject(
    ECSHelper* Factory, CreateUnitProducingGameObjectEvent Event) {
  auto Entity = CreateGameObject(
      Factory, CreateGameObjectEvent(Event.NodeName, Event.MeshName,
                                     Event.EntityName, Event.InitialPosition,
                                     Event.GamePlayer, Event.PlayerId));

  Factory->FactoryQueue->Enqueue<AddUnitProductionEvent>(
      AddUnitProductionEvent(Entity, Event.UnitProdPerM));

  return Entity;
}

// template to produce an attacking entity
inline std::shared_ptr<entt::entity> CreateAttackingGameObject(
    ECSHelper* Factory, CreateAttackingEntityEvent Event) {
  auto Entity = CreateGameObject(
      Factory, CreateGameObjectEvent(Event.NodeName, Event.MeshName,
                                     Event.EntityName, Event.InitialPosition,
                                     Event.GamePlayer, Event.PlayerId));
  Factory->FactoryQueue->Enqueue<AddHealthEvent>(
      AddHealthEvent(Entity, Event.Health));
  Factory->FactoryQueue->Enqueue<AddAttackEvent>(
      AddAttackEvent(Entity, Event.Radius, Event.Power));
  Factory->FactoryQueue->Enqueue<AddRangeComponentEvent>(
      AddRangeComponentEvent(Entity));

  return Entity;
}

}  // namespace EntityTemplates
