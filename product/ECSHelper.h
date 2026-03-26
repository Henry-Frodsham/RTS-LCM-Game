// Copyright © 2025 Henry Frodsham
#pragma once
#include <entt/entt.hpp>

#include "BaseComponents.h"
#include "CommonGameObjectComponents.h"
#include "CityComponents.h"
#include "ECSFactoryEvent.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "RenderSystem.h"

// creates ECS entities
// seperated from WorldManager to consider future implementation of multiple ECS
// registries this class doesnt wrap entt::registry itself or change its
// behaviour, it simply contains construction templates for common components
class ECSHelper {
 public:
  ECSHelper(entt::registry* Registry, ErrorReporter* Reporter);
  ECSHelper() = default;

  template <typename T>
  T* TryGetComponent(entt::entity entity) {
    return RegistryToUse->try_get<T>(entity);
  }

  EventQueue* FactoryQueue;
  void ChangeEntityVisibility(ChangeEntityVisibilityEvent Event);
  void MoveEntityAlongSpherical(MoveEntityAlongSphericalEvent Event);
 private:
  EventBus* FactoryBus;

  ErrorReporter* ParentReporter;

  entt::registry* RegistryToUse;

  void CreateAndAddEntity(CreateEntityEvent Event);

  void CreateAndAddOgreComponent(AddOgreComponentEvent Event);

  void CreateAndAddMeshComponent(AddMeshComponentEvent Event);

  void CreateAndAddOwnerShipComponent(AddOwnerShipComponentEvent Event);

  void CreateandAddUnitProductionComponent(AddUnitProductionEvent Event);
  
  void OrientateAndAdditionalSetup(OrientateEntityEvent Event);
  OgreComponent FindEntityFromSceneNodeName(std::string NodeName);
  
};
