// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <entt/entt.hpp>
#include <optional>  // NOLINT(build/include_order)
#include <string>  // NOLINT(build/include_order)
#include <utility>  // NOLINT(build/include_order)

#include "BaseComponents.h"
#include "CityComponents.h"
#include "CommonGameObjectComponents.h"
#include "ECSFactoryEvent.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "RenderQueryFlags.h"
#include "RenderSystem.h"
#include "TraversalComponents.h"
#include "UnitComponents.h"

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

  void UpdateAndColludeEntityRanges(EntitiesInRangeUpdateEvent Event);

 private:
  EventBus* FactoryBus;

  ErrorReporter* ParentReporter;

  entt::registry* RegistryToUse;

  void CreateAndAddEntity(CreateEntityEvent Event);

  void CreateAndAddOgreComponent(AddOgreComponentEvent Event);

  void CreateAndAddMeshComponent(AddMeshComponentEvent Event);

  void CreateAndAddOwnerShipComponent(AddOwnerShipComponentEvent Event);

  void CreateandAddUnitProductionComponent(AddUnitProductionEvent Event);

  void CreateandAddAttackComponent(AddMeleeAttackEvent Event);

  void CreateandAddHealthComponent(AddHealthEvent Event);

  void CreateAndAddRangeComponent(AddRangeComponentEvent Event);

  void CreateAndAddExistableComponent(AddExistableComponentEvent Event);

  void CreateAndAddMovableComponent(AddMovableComponentEvent Event);

  void IssueRangeRevalEvent(NotifyConsequentialEntityStateChange Notif);

  void OrientateAndAdditionalSetup(OrientateEntityEvent Event);

  void ValidateEntitySelection(TrySelectEntityEvent Event);

  void ValidateEntityMovement(TryMoveEntityEvent Event);

  void RequestPathPreview(RequestPathPreviewEvent Event);

  // resolves the tile-graph start/goal for a move (real or preview) given a
  // destination world position and the entity's current MeshComponent
  // position - shared by ValidateEntityMovement and RequestPathPreview since
  // both need the identical GlobeInterface round-trip
  std::optional<std::pair<uint32_t, uint32_t>> ResolveMoveTiles(
      MeshComponent* MeshComp, Ogre::Vector3f DestinationWorldPos);

  void TryUnselectEntity(TryUnselectEntityEvent Event);

  void TryDestroyEntity(TryDestroyEntityEvent Event);


  OgreComponent FindEntityFromSceneNodeName(std::string NodeName);

  entt::entity FindEntityFromSceneNode(Ogre::SceneNode* Node);
};
