// Copyright (c) 2025 Henry Frodsham
#include "WorldManager.h"

#include <cmath>
#include <unordered_map>
#include <vector>

#include "Player.h"
#include "Pathfinder.h"

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
  AdvancePathRequests();
  AdvancePathPreviewRequests();
  AdvanceMovingEntities(DT);
}

void WorldManager::AdvancePathRequests() {
  GlobeInterface* GInt = RenderSystem::GetInstance().GetGlobeInterface();
  const Globe* WorldGlobe = GInt->GetGlobe();

  auto View = Registry.view<PathRequestComponent, MovableComponent>();

  // requests can't be removed from the registry mid-view, so collect
  // resolved entities and remove their PathRequestComponent afterwards
  std::vector<entt::entity> Resolved;

  for (auto [Entity, Request, Mover] : View.each()) {
    const PathStatus Status =
        Pathfinder::Step(Request.Search, *WorldGlobe, Mover.MovableBiomes,
                         Pathfinder::kMaxExpansionsPerTick);

    if (Status == PathStatus::Found) {
      Mover.Path = Pathfinder::ReconstructPath(Request.Search);
      Resolved.push_back(Entity);
    } else if (Status == PathStatus::Failed) {
      ECSReporter->EnqueueError(
          ErrorDetail::CreateError(ErrorCode::PATH_DESTINATION_UNREACHABLE));
      Resolved.push_back(Entity);
    }
    // PathStatus::InProgress - leave the component for next tick's budget
  }

  for (entt::entity Entity : Resolved) {
    Registry.remove<PathRequestComponent>(Entity);
  }
}

void WorldManager::AdvancePathPreviewRequests() {
  GlobeInterface* GInt = RenderSystem::GetInstance().GetGlobeInterface();
  const Globe* WorldGlobe = GInt->GetGlobe();

  auto View = Registry.view<PathPreviewRequestComponent, MovableComponent,
                            MeshComponent, OwnershipComponent>();
  std::vector<entt::entity> Resolved;

  for (auto [Entity, Request, Mover, MeshComp, Ownership] : View.each()) {
    const PathStatus Status =
        Pathfinder::Step(Request.Search, *WorldGlobe, Mover.MovableBiomes,
                         Pathfinder::kMaxExpansionsPerTick);

    if (Status == PathStatus::Found) {
      Mover.PreviewPath = Pathfinder::ReconstructPath(Request.Search);
      Resolved.push_back(Entity);

      std::vector<Ogre::Vector3f> Points;
      Points.reserve(Mover.PreviewPath.size() + 1);
      Points.push_back(MeshComp.Entity->getParentSceneNode()->getPosition());
      for (uint32_t TileID : Mover.PreviewPath) {
        Points.push_back(GInt->GetWorldPositionForTile(TileID));
      }

      RenderSystem::GetInstance().RenderQueue->Enqueue(
          UpdatePathPreviewEvent(Points, true, Ownership.PlayerID));
    } else if (Status == PathStatus::Failed) {
      // unlike a real move, a preview that can't find a route is expected
      // and constant while dragging over water/mountains - no error report
      Mover.PreviewPath.clear();
      Resolved.push_back(Entity);
      RenderSystem::GetInstance().RenderQueue->Enqueue(
          UpdatePathPreviewEvent({}, false));
    }
    // PathStatus::InProgress - leave the component for next tick's budget
  }

  for (entt::entity Entity : Resolved) {
    Registry.remove<PathPreviewRequestComponent>(Entity);
  }
}

void WorldManager::AdvanceMovingEntities(float DT) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  GlobeInterface* GInt = Rs.GetGlobeInterface();

  constexpr float kArrivalEpsilonSq = 1e-6f;

  auto View = Registry.view<MovableComponent, MeshComponent>();
  for (auto [Entity, Mover, MeshComp] : View.each()) {
    if (!Mover.IsMoving() || !MeshComp.Entity) {
      continue;
    }

    Ogre::SceneNode* SN = MeshComp.Entity->getParentSceneNode();
    if (!SN) {
      continue;
    }

    Ogre::Vector3 CurrentPos = SN->getPosition();
    float Budget = Mover.Speed * DT;

    while (Budget > 0.f && !Mover.Path.empty()) {
      const uint32_t NextTile = Mover.Path.front();
      const Ogre::Vector3 TargetPos = GInt->GetWorldPositionForTile(NextTile);
      const Ogre::Vector3 Delta = TargetPos - CurrentPos;
      const float DistSq = Delta.squaredLength();

      if (DistSq <= kArrivalEpsilonSq || std::sqrt(DistSq) <= Budget) {
        CurrentPos = TargetPos;
        Budget -= std::sqrt(DistSq);
        Mover.Path.pop_front();
        Rs.RenderQueue->Enqueue(RotateEntToSurfaceNormalEvent(
            MeshComp.Entity, GInt->GetWorldNormalForTile(NextTile)));
      } else {
        CurrentPos += Delta * (Budget / std::sqrt(DistSq));
        Budget = 0.f;
      }
    }

    Rs.RenderQueue->Enqueue(SetEntPositionEvent(MeshComp.Entity, CurrentPos));

    if (Mover.Path.empty()) {
      CompFactory->FactoryQueue->Enqueue(
          NotifyConsequentialEntityStateChange(MeshComp.Entity));
    }
  }
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

  auto AttackView = Registry.view<HealthComponent, MeleeAttackComponent,
                                  RangeCacheComponent, OgreComponent>();

  for (auto Entity : AttackView) {
    auto& Range = AttackView.get<RangeCacheComponent>(Entity);
    auto& Attack = AttackView.get<MeleeAttackComponent>(Entity);
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
