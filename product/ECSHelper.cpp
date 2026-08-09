// Copyright (c) 2025 Henry Frodsham
#include "ECSHelper.h"

#include <string>

#include "RenderEvent.h"

// helper class responsible for creating components
// however, doesnt manage or evaluate components
// (thats worldmanagers job)
ECSHelper::ECSHelper(entt::registry* Registry, ErrorReporter* Reporter)
    : RegistryToUse(Registry), ParentReporter(Reporter) {
  FactoryBus = new EventBus();
  FactoryQueue = new EventQueue(FactoryBus);

  FactoryBus->Subscribe<CreateEntityEvent>(
      std::bind(&ECSHelper::CreateAndAddEntity, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddOgreComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddOgreComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddMeshComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddMeshComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddOwnerShipComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddOwnerShipComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<MoveEntityAlongSphericalEvent>(std::bind(
      &ECSHelper::MoveEntityAlongSpherical, this, std::placeholders::_1));
  FactoryBus->Subscribe<OrientateEntityEvent>(std::bind(
      &ECSHelper::OrientateAndAdditionalSetup, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddUnitProductionEvent>(
      std::bind(&ECSHelper::CreateandAddUnitProductionComponent, this,
                std::placeholders::_1));
  FactoryBus->Subscribe<AddHealthEvent>(std::bind(
      &ECSHelper::CreateandAddHealthComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddMeleeAttackEvent>(std::bind(
      &ECSHelper::CreateandAddAttackComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddRangeComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddRangeComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddExistableComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddExistableComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddMovableComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddMovableComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<NotifyConsequentialEntityStateChange>(
      std::bind(&ECSHelper::IssueRangeRevalEvent, this, std::placeholders::_1));
  FactoryBus->Subscribe<EntitiesInRangeUpdateEvent>(std::bind(
      &ECSHelper::UpdateAndColludeEntityRanges, this, std::placeholders::_1));
  FactoryBus->Subscribe<TrySelectEntityEvent>(std::bind(
      &ECSHelper::ValidateEntitySelection, this, std::placeholders::_1));
  FactoryBus->Subscribe<TryMoveEntityEvent>(std::bind(
      &ECSHelper::ValidateEntityMovement, this, std::placeholders::_1));
  FactoryBus->Subscribe<RequestPathPreviewEvent>(std::bind(
      &ECSHelper::RequestPathPreview, this, std::placeholders::_1));
  FactoryBus->Subscribe<TryUnselectEntityEvent>(
      std::bind(&ECSHelper::TryUnselectEntity, this, std::placeholders::_1));
  FactoryBus->Subscribe<TryDestroyEntityEvent>(
      std::bind(&ECSHelper::TryDestroyEntity, this, std::placeholders::_1));
}

void ECSHelper::CreateAndAddEntity(CreateEntityEvent Event) {
  *Event.CreatedEntity = RegistryToUse->create();
}

void ECSHelper::CreateAndAddOgreComponent(AddOgreComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  auto& Component = RegistryToUse->emplace<OgreComponent>(
      *Event.Entity, nullptr, Event.NodeName);

  Rs.RenderQueue->Enqueue(
      CreateSceneNodeEvent(std::ref(Component.EntityNode), Event.NodeName));

  Rs.RenderQueue->Enqueue(SetNodePositionEvent(std::ref(Component.EntityNode),
                                               Event.InitialPosition));
}

void ECSHelper::CreateAndAddMeshComponent(AddMeshComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Ogre::SceneNode* NodeToAttach = nullptr;
  try {
    OgreComponent& EntOgreComp =
        RegistryToUse->get<OgreComponent>(*Event.Entity);
    auto& Component = RegistryToUse->emplace<MeshComponent>(
        *Event.Entity, nullptr, Event.MeshName, Event.EntityName);

    Rs.RenderQueue->Enqueue(CreateOgreEntityEvent(
        std::ref(Component.Entity), Event.EntityName, Event.MeshName,
        *Event.Entity, RenderQueryFlags::kSelectableUnit));

    Rs.RenderQueue->Enqueue(AttachEntityToScenNodeEvent(
        std::ref(Component.Entity), std::ref(EntOgreComp.EntityNode)));
  } catch (std::exception& e) {
    ParentReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ECS_WRONG_CREATION_ORDER,
        "failed to create MeshComponent due to this entity not having a "
        "OgreComponent to attach to"));
    return;
  }
}

void ECSHelper::CreateandAddUnitProductionComponent(
    AddUnitProductionEvent Event) {
  auto& Component = RegistryToUse->emplace<ProducesUnitsComponent>(
      *Event.Entity, Event.ProdPerM);
}
void ECSHelper::CreateandAddHealthComponent(AddHealthEvent Event) {
  auto& Component =
      RegistryToUse->emplace<HealthComponent>(*Event.Entity, Event.Health);
}
void ECSHelper::CreateandAddAttackComponent(AddMeleeAttackEvent Event) {
  auto& Component = RegistryToUse->emplace<MeleeAttackComponent>(
      *Event.Entity, Event.Damage);
}
void ECSHelper::ChangeEntityVisibility(ChangeEntityVisibilityEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  try {
    OgreComponent Comp = FindEntityFromSceneNodeName(Event.NodeName);
    if (Comp.EntityNode != nullptr) {
      Comp.EntityNode->setVisible(Event.Visible);
    }
  } catch (std::exception& e) {
    ParentReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ECS_WRONG_CREATION_ORDER,
        "failed to create MeshComponent due to this entity not having a "
        "OgreComponent to attach to"));
    return;
  }
}

void ECSHelper::MoveEntityAlongSpherical(MoveEntityAlongSphericalEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Ogre::Vector3 Pos = Event.Unit->getParentSceneNode()->getPosition();

  Ogre::Vector3 globeCenter = Ogre::Vector3(0.5f, 0.f, -5.f);
  float radius = (Pos - globeCenter).length();

  Ogre::Vector3 newDir = (Event.TargetPos - globeCenter).normalisedCopy();
  Ogre::Vector3 newPos = globeCenter + newDir * radius;

  Rs.RenderQueue->Enqueue(SetEntPositionEvent(Event.Unit, newPos));
}

void ECSHelper::CreateAndAddOwnerShipComponent(
    AddOwnerShipComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  try {
    OgreComponent& EntOgreComp =
        RegistryToUse->get<OgreComponent>(*Event.Entity);

    if (RegistryToUse->try_get<OwnershipComponent>(*Event.Entity) == nullptr) {
      auto& Component = RegistryToUse->emplace<OwnershipComponent>(
          *Event.Entity, Event.PlayerID, Event.GamePlayer);
    }

    // ordering is very tricky for this, because when this runs the first time
    // the scenenode and entity havent been created good news is we can just
    // callback this and run it again until it works
    if (EntOgreComp.EntityNode != nullptr) {
      Rs.RenderQueue->Enqueue(
          AddOwnerShipToEntEvent(EntOgreComp.EntityNode, Event.PlayerID));
    } else {
      FactoryQueue->Enqueue(Event);
    }
  } catch (std::exception e) {
  }
}

void ECSHelper::CreateAndAddRangeComponent(AddRangeComponentEvent Event) {
  std::set<entt::entity> EmptyRangeContainer;
  auto& Component = RegistryToUse->emplace<RangeCacheComponent>(
      *Event.Entity, EmptyRangeContainer);
}



void ECSHelper::CreateAndAddExistableComponent(
    AddExistableComponentEvent Event) {
  auto& Component = RegistryToUse->emplace<ExistableComponent>(
      *Event.Entity, Event.ExistableBiomes);
}

void ECSHelper::CreateAndAddMovableComponent(AddMovableComponentEvent Event) {
  auto& Component = RegistryToUse->emplace<MovableComponent>(
      *Event.Entity, Event.MovableBiomes, Event.MoveSpeed);
}

void ECSHelper::OrientateAndAdditionalSetup(OrientateEntityEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  try {
    MeshComponent* EntOgreComp =
        RegistryToUse->try_get<MeshComponent>(*Event.Entity);

    if (EntOgreComp->Entity != nullptr) {
      Rs.RenderQueue->Enqueue(RotateEntToSurfaceNormalEvent(
          EntOgreComp->Entity, Event.SurfaceNormal));
      FactoryQueue->Enqueue(
          NotifyConsequentialEntityStateChange(EntOgreComp->Entity));
    } else {
      FactoryQueue->Enqueue(Event);
    }
  } catch (std::exception e) {
  }
}

void ECSHelper::ValidateEntitySelection(TrySelectEntityEvent Event) {
  if (!RegistryToUse->valid(Event.Entity) || !Event.TryingPlayer) {
    return;
  }
  OwnershipComponent* OComp =
      RegistryToUse->try_get<OwnershipComponent>(Event.Entity);
  MeshComponent* MeshComp = RegistryToUse->try_get<MeshComponent>(Event.Entity);
  if (!OComp || !MeshComp) {
    return;
  }

  if (OComp->GamePlayer == Event.TryingPlayer) {
    // success
    RenderSystem& Rs = RenderSystem::GetInstance();
    Rs.RenderQueue->Enqueue(
        ChangeEntMaterialEvent(MeshComp->Entity, "RED_UNIT"));
    Event.Respond();
  }
}

void ECSHelper::ValidateEntityMovement(TryMoveEntityEvent Event) {
  if (!RegistryToUse->valid(Event.Entity) || !Event.TryingPlayer) {
    return;
  }
  OwnershipComponent* OComp =
      RegistryToUse->try_get<OwnershipComponent>(Event.Entity);
  MeshComponent* MeshComp = RegistryToUse->try_get<MeshComponent>(Event.Entity);
  MovableComponent* MovComp =
      RegistryToUse->try_get<MovableComponent>(Event.Entity);
  if (!OComp || !MeshComp || !MovComp) {
    return;
  }
  if (!MovComp->MovableBiomes.at(Event.SelectedBiome)) {
    return;
  }
  if (OComp->GamePlayer == Event.TryingPlayer) {
    // success - resolve the tile-graph endpoints and hand off to the
    // incremental pathfinder rather than teleporting directly. The actual
    // walk happens in WorldManager::AdvanceMovingEntities once a path is
    // found (WorldManager::AdvancePathRequests)
    const auto Tiles = ResolveMoveTiles(MeshComp, Event.Position);
    if (!Tiles) {
      ParentReporter->EnqueueError(
          ErrorDetail::CreateError(ErrorCode::PATH_INVALID_TILE));
      return;
    }
    const auto [StartTile, GoalTile] = *Tiles;

    // a new move order stops the unit where it is and re-routes, rather
    // than letting it glide on the old path while the new search resolves.
    // also drops any in-flight preview so it can't linger once the unit is
    // actually walking a different path
    MovComp->Path.clear();
    MovComp->PreviewPath.clear();
    RegistryToUse->remove<PathPreviewRequestComponent>(Event.Entity);
    RenderSystem::GetInstance().RenderQueue->Enqueue(
        UpdatePathPreviewEvent({}, false));

    RegistryToUse->emplace_or_replace<PathRequestComponent>(
        Event.Entity, GoalTile,
        Pathfinder::BeginSearch(*RenderSystem::GetInstance()
                                     .GetGlobeInterface()
                                     ->GetGlobe(),
                                StartTile, GoalTile));
  }
}

void ECSHelper::RequestPathPreview(RequestPathPreviewEvent Event) {
  if (!RegistryToUse->valid(Event.Entity) || !Event.TryingPlayer) {
    return;
  }
  OwnershipComponent* OComp =
      RegistryToUse->try_get<OwnershipComponent>(Event.Entity);
  MeshComponent* MeshComp = RegistryToUse->try_get<MeshComponent>(Event.Entity);
  MovableComponent* MovComp =
      RegistryToUse->try_get<MovableComponent>(Event.Entity);
  if (!OComp || !MeshComp || !MovComp) {
    return;
  }
  if (!MovComp->MovableBiomes.at(Event.SelectedBiome)) {
    return;
  }
  if (OComp->GamePlayer != Event.TryingPlayer) {
    return;
  }

  // unlike a real move order, a preview missing its tiles is a constant,
  // expected occurrence while dragging over water/mountains - not an error
  const auto Tiles = ResolveMoveTiles(MeshComp, Event.Position);
  if (!Tiles) {
    return;
  }
  const auto [StartTile, GoalTile] = *Tiles;

  RegistryToUse->emplace_or_replace<PathPreviewRequestComponent>(
      Event.Entity, GoalTile,
      Pathfinder::BeginSearch(
          *RenderSystem::GetInstance().GetGlobeInterface()->GetGlobe(),
          StartTile, GoalTile));
}

std::optional<std::pair<uint32_t, uint32_t>> ECSHelper::ResolveMoveTiles(
    MeshComponent* MeshComp, Ogre::Vector3f DestinationWorldPos) {
  GlobeInterface* GInt = RenderSystem::GetInstance().GetGlobeInterface();
  const uint32_t GoalTile = GInt->FindTileAtWorldPosition(DestinationWorldPos);
  const uint32_t StartTile = GInt->FindTileAtWorldPosition(
      MeshComp->Entity->getParentSceneNode()->getPosition());

  if (GoalTile == InvalidTileID || StartTile == InvalidTileID) {
    return std::nullopt;
  }
  return std::make_pair(StartTile, GoalTile);
}

void ECSHelper::TryUnselectEntity(TryUnselectEntityEvent Event) {
  if (!RegistryToUse->valid(Event.Entity)) {
    return;
  }
  MeshComponent* MeshComp = RegistryToUse->try_get<MeshComponent>(Event.Entity);
  if (!MeshComp) {
    return;
  }
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeEntMaterialEvent(MeshComp->Entity, "WHITE"));
}

void ECSHelper::TryDestroyEntity(TryDestroyEntityEvent Event) {
  if (!RegistryToUse->valid(Event.Entity)) {
    return;
  }
  MeshComponent* MeshComp = RegistryToUse->try_get<MeshComponent>(Event.Entity);
  if (!MeshComp) {
    return;
  }
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(DestroyEntityEvent(MeshComp->Entity));
}

void ECSHelper::UpdateAndColludeEntityRanges(EntitiesInRangeUpdateEvent Event) {
  auto RangeView =
      RegistryToUse->view<HealthComponent, OwnershipComponent,
                          OgreComponent, RangeCacheComponent>();

  std::vector<entt::entity> RelevantEntities;
  entt::entity EntityOfInterest = FindEntityFromSceneNode(Event.OriginalNode);
  RangeCacheComponent* RangeCompToUpdate =
      RegistryToUse->try_get<RangeCacheComponent>(EntityOfInterest);
  OwnershipComponent* EntityOfInterestOwnership =
      RegistryToUse->try_get<OwnershipComponent>(EntityOfInterest);

  if (!RangeCompToUpdate || !EntityOfInterestOwnership) {
    return;
  }
  for (auto Entity : RangeView) {
    auto& Health = RangeView.get<HealthComponent>(Entity);
    auto& Ownership = RangeView.get<OwnershipComponent>(Entity);
    auto& Ogre = RangeView.get<OgreComponent>(Entity);
    auto& Range = RangeView.get<RangeCacheComponent>(Entity);

    if (!Event.EntitiesInRange.contains(Ogre.EntityNode)) {
      continue;
    }

    if (Ownership.PlayerID != EntityOfInterestOwnership->PlayerID) {
      RelevantEntities.push_back(Entity);
      // allow the entity in range to also attack this entity, since the other
      // entity may not have updated
      Range.EntitiesInRange.insert(EntityOfInterest);
    }
  }
  RangeCompToUpdate->EntitiesInRange =
      std::set<entt::entity>(RelevantEntities.begin(), RelevantEntities.end());
}

void ECSHelper::IssueRangeRevalEvent(
    NotifyConsequentialEntityStateChange Notif) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(
      RevalEntityRangeEvent(Notif.Entity, FactoryQueue, 10.f));
}

OgreComponent ECSHelper::FindEntityFromSceneNodeName(std::string NodeName) {
  auto View = RegistryToUse->view<OgreComponent>();
  for (auto Entity : View) {
    OgreComponent EntComp = View.get<OgreComponent>(Entity);

    if (EntComp.NodeName == NodeName) {
      return EntComp;
    }
  }
  return OgreComponent(nullptr, "");
}

entt::entity ECSHelper::FindEntityFromSceneNode(Ogre::SceneNode* Node) {
  auto View = RegistryToUse->view<OgreComponent>();
  for (auto Entity : View) {
    OgreComponent EntComp = View.get<OgreComponent>(Entity);

    if (EntComp.EntityNode == Node) {
      return Entity;
    }
  }
  return entt::null;
}
