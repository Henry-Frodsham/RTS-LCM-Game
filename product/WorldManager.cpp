// Copyright © 2025 Henry Frodsham
#include "WorldManager.h"

#include "Player.h"

WorldManager::WorldManager(bool CreateGlobe) {
  WorldBus = new EventBus();
  WorldQueue = new EventQueue(WorldBus);
  ECSReporter = new ErrorReporter();

  CompFactory = new ECSHelper(&Registry, ECSReporter);

  Evaluator = new EntityInteractionEvaluator();
  // dont subscribe to the CompFactory functions directly for initial entity
  // creation since it requires multiple steps so let templates handle that
  // subscribe using a lambda instead of std::bind since entity template
  // functions need the factory too
  WorldBus->Subscribe<CreateBasicWorldEntityEvent>(
      [this](const CreateBasicWorldEntityEvent& Event) {
        EntityTemplates::ConstructBasicEntity(CompFactory, Event);
      });

  WorldBus->Subscribe<CreateMeshWorldEntityEvent>(
      [this](const CreateMeshWorldEntityEvent& Event) {
        EntityTemplates::ConstructMeshEntity(CompFactory, Event);
      });

  WorldBus->Subscribe<ChangeEntityVisibilityEvent>(
      [this](const ChangeEntityVisibilityEvent& Event) {
        CompFactory->ChangeEntityVisibility(Event);
      });
  WorldBus->Subscribe<ChangeGlobeVisibilityEvent>(std::bind(
      &WorldManager::ChangeGlobeVisibility, this, std::placeholders::_1));
  WorldBus->Subscribe<CachedEntitiesReturnEvent>(
      std::bind(&WorldManager::UpdateRangeCache, this, std::placeholders::_1));
  WorldBus->Subscribe<MoveEntityAlongSphericalEvent>(
      std::bind(&ECSHelper::MoveEntityAlongSpherical, CompFactory,
                std::placeholders::_1));
  if (CreateGlobe) {
    CreateGlobeMesh();
  }
}

void WorldManager::CreateGlobeMesh() {
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "PlanetBase.mesh", "GlobeBase",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "antartica_n.mesh", "GlobeAntarticaN",
      Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "antartica_s.mesh", "GlobeAntarticaS",
      Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "europa.mesh", "GlobeEuropa",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "greenland.mesh", "GlobeGreenland",
      Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "iceland.mesh", "GlobeIceland",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "UK.mesh", "GlobeUK", Ogre::Vector3(0.5f, 0.f, -5.f)));
}

void WorldManager::ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event) {
  WorldQueue->Enqueue(ChangeEntityVisibilityEvent("GlobeNode", Event.Visible));
}
void WorldManager::update(float DT) {
  ECSReporter->Dispatch();
  WorldQueue->Dispatch();
  CompFactory->FactoryQueue->Dispatch();
  EvaluateTickerComponents(DT);
}

void WorldManager::EvaluateTickerComponents(float DT) {
  // Unit production
  std::unordered_map<int, std::vector<entt::entity>> PlayerProductionMap;

  auto View = Registry.view<ProducesUnitsComponent, OwnershipComponent>();

  for (auto [Entity, Produces, Ownership] : View.each()) {
    PlayerProductionMap[Ownership.PlayerID].push_back(Entity);
  }

  for (auto& [PlayerID, Entities] : PlayerProductionMap) {
    for (entt::entity Entity : Entities) {
      auto& Produces = View.get<ProducesUnitsComponent>(Entity);
      auto& Ownership = View.get<OwnershipComponent>(Entity);

      const float ProgressPerSecond = (Produces.NumPerMinute / 60.f) * 100.f;
      Ownership.GamePlayer->PlayerQueue->Enqueue(
          UpdateUnitProgressEvent(ProgressPerSecond * DT));
    }
  }
  // Combat evaluation

  std::unordered_map<Ogre::SceneNode*, OwnershipComponent> ToCache;
  std::unordered_map<int, std::vector<entt::entity>> PlayerCombatMap;
  std::unordered_map<Ogre::SceneNode*, entt::entity> NodeMap;

  auto CombatView = Registry.view<HealthComponent, AttackComponent,
                                  OwnershipComponent, OgreComponent>();

  RenderSystem& Rs = RenderSystem::GetInstance();

  for (auto Entity : CombatView) {
    auto& Ownership = CombatView.get<OwnershipComponent>(Entity);
    auto& Ogre = CombatView.get<OgreComponent>(Entity);
    PlayerCombatMap[Ownership.PlayerID].push_back(Entity);
    NodeMap[Ogre.EntityNode] = Entity;
  }
  for (auto& [PlayerID, Entities] : PlayerCombatMap) {
    for (entt::entity Entity : Entities) {
      if (!Registry.valid(Entity)) continue;

      auto& Health = CombatView.get<HealthComponent>(Entity);
      auto& Attack = CombatView.get<AttackComponent>(Entity);
      auto& Ownership = CombatView.get<OwnershipComponent>(Entity);
      auto& Ogre = CombatView.get<OgreComponent>(Entity);

      auto CacheIt = CachedRangeEntities.find(Ogre.EntityNode);
      if (CacheIt == CachedRangeEntities.end()) {
        ToCache.emplace(Ogre.EntityNode, Ownership);
        continue;
      }

      for (Ogre::SceneNode* Node : CacheIt->second) {
        auto NodeIt = NodeMap.find(Node);
        if (NodeIt == NodeMap.end()) continue;

        entt::entity RangeEntity = NodeIt->second;
        if (!Registry.valid(RangeEntity)) continue;

        auto& RangeHealth = CombatView.get<HealthComponent>(RangeEntity);
        auto& RangeOwnership = CombatView.get<OwnershipComponent>(RangeEntity);
        auto& RangeOgre = CombatView.get<OgreComponent>(RangeEntity);

        if (RangeOwnership.PlayerID == PlayerID) continue;

        // attack logic, RangeEntity is to be attacked by Entity

        Evaluator->ProcessAttackEvent(
            AttackEvent(Attack.Damage, &RangeHealth.Health, DT));

        if (RangeHealth.Health <= 0) {
          Rs.RenderQueue->Enqueue(DestroyNodeEvent(RangeOgre.EntityNode));

          NodeMap.erase(RangeOgre.EntityNode);
          CachedRangeEntities.erase(RangeOgre.EntityNode);
          ToCache.erase(RangeOgre.EntityNode);

          Registry.destroy(RangeEntity);
        }
      }

      ToCache.emplace(Ogre.EntityNode, Ownership);
    }
  }

  if (ToCache.size() != 0) {
    // theres 0 chance i can get entity positions thread safely and on demand
    // so ive decided to cache in range units ahead of time.
    Rs.RenderQueue->Enqueue(CacheRangeQueryEvent(0.01f, ToCache, WorldQueue));
  }
}

void WorldManager::UpdateRangeCache(CachedEntitiesReturnEvent Event) {
  CachedRangeEntities = Event.Entities;
}
