// Copyright (c) 2025 Henry Frodsham
#include "WorldManager.h"

#include <unordered_map>
#include <vector>

#include "Player.h"

WorldManager::WorldManager() {
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
  WorldBus->Subscribe<MoveEntityAlongSphericalEvent>(
      std::bind(&ECSHelper::MoveEntityAlongSpherical, CompFactory,
                std::placeholders::_1));
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

  // combat evaluation
  RenderSystem& Rs = RenderSystem::GetInstance();

  auto AttackView = Registry.view<HealthComponent, AttackComponent,
                                  RangeCacheComponent, OgreComponent>();

  for (auto Entity : AttackView) {
    auto& Range = AttackView.get<RangeCacheComponent>(Entity);
    auto& Attack = AttackView.get<AttackComponent>(Entity);
    auto& Health = AttackView.get<HealthComponent>(Entity);
    auto& Ogre = AttackView.get<OgreComponent>(Entity);

    if (!Ogre.EntityNode) {
      ECSReporter->EnqueueError(
          ErrorDetail::CreateError(ErrorCode::ATTACK_LOGIC_EARLY));
    }

    std::erase_if(Range.EntitiesInRange,
                  [&](entt::entity e) { return !Registry.valid(e); });

    for (auto EntInRange : Range.EntitiesInRange) {
      if (!Registry.valid(EntInRange)) {
        continue;
      }
      auto& EntHealth = Registry.get<HealthComponent>(EntInRange);
      auto& EntOgre = Registry.get<OgreComponent>(EntInRange);

      if (!EntOgre.EntityNode) {
        ECSReporter->EnqueueError(
            ErrorDetail::CreateError(ErrorCode::ATTACK_LOGIC_EARLY));
      }

      try {
        Evaluator->ProcessAttackEvent(
            AttackEvent(Attack.Damage, &EntHealth.Health, DT));

        if (EntHealth.Health <= 0) {
          Rs.RenderQueue->Enqueue(DestroyNodeEvent(EntOgre.EntityNode));

          Registry.destroy(EntInRange);
        }
      }

      catch (std::exception& e) {
        ECSReporter->EnqueueError(
            ErrorDetail::CreateError(ErrorCode::ATTACK_LOGIC_FAILED));
      }
    }
  }
}
