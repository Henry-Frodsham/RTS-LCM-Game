// Copyright (c) 2025 Henry Frodsham
#include "WorldManager.h"

#include <cmath>
#include <unordered_map>
#include <vector>

#include "Player.h"
#include "Pathfinder.h"

namespace {
// cosine of the half-angle of the frontal arc an attacker must face a
// target within for an attack to land. 0.f = a 90 degree half-angle either
// side of Forward, i.e. the whole front hemisphere counts - tune (or turn
// into a damage curve instead of an all-or-nothing gate) once "units attack
// most effectively at the front" gets real balancing
constexpr float kFrontArcCosHalfAngle = 0.f;

// secondary check performed only at attack time, alongside (not instead of)
// RangeCacheComponent's sphere-based membership check - a target can be a
// valid, in-range enemy without being in front of the attacker
bool IsTargetWithinFrontArc(const Ogre::Vector3f& Forward,
                            const Ogre::Vector3& AttackerPos,
                            const Ogre::Vector3& TargetPos) {
  Ogre::Vector3 ToTarget = TargetPos - AttackerPos;
  if (ToTarget.squaredLength() < 1e-8f) {
    return true;
  }
  ToTarget.normalise();
  return Forward.dotProduct(ToTarget) >= kFrontArcCosHalfAngle;
}
}  // namespace

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

    FacingComponent* Facing = Registry.try_get<FacingComponent>(Entity);
    FacingArrowComponent* Arrow = Registry.try_get<FacingArrowComponent>(Entity);

    Ogre::Vector3 CurrentPos = SN->getPosition();
    float Budget = Mover.Speed * DT;

    while (Budget > 0.f && !Mover.Path.empty()) {
      const uint32_t NextTile = Mover.Path.front();
      const Ogre::Vector3 TargetPos = GInt->GetWorldPositionForTile(NextTile);
      const Ogre::Vector3 Delta = TargetPos - CurrentPos;
      const float DistSq = Delta.squaredLength();

      if (Facing && DistSq > kArrivalEpsilonSq) {
        // project onto the tangent plane of the unit's current surface
        // normal (local +Y post-RotateEntityToSurfaceNormal) so Forward
        // stays a unit-length tangent vector usable for the front-arc
        // attack check, not just a raw chord direction
        const Ogre::Vector3 CurrentNormal =
            SN->getOrientation() * Ogre::Vector3::UNIT_Y;
        const Ogre::Vector3 Tangential =
            Delta - CurrentNormal * Delta.dotProduct(CurrentNormal);
        if (Tangential.squaredLength() > kArrivalEpsilonSq) {
          Facing->Forward = Tangential.normalisedCopy();
          if (Arrow) {
            Rs.RenderQueue->Enqueue(UpdateFacingArrowOrientationEvent(
                Arrow->ArrowNode, Facing->Forward, CurrentNormal));
          }
        }
      }

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

    CompFactory->FactoryQueue->Enqueue(
        NotifyConsequentialEntityStateChange(MeshComp.Entity));
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
    FacingComponent* Facing = Registry.try_get<FacingComponent>(Entity);

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

      // secondary front-arc check - RangeCacheComponent above still governs
      // what counts as "in range" for targeting; this additionally requires
      // the attacker to be facing roughly towards the target before any
      // damage lands. falls back to always-allow if either node is missing
      // or the attacker has no FacingComponent, so this can never newly
      // block an attack that would previously have landed unconditionally
      if (Facing && Ogre.EntityNode && EntOgre.EntityNode &&
          !IsTargetWithinFrontArc(Facing->Forward,
                                  Ogre.EntityNode->_getDerivedPosition(),
                                  EntOgre.EntityNode->_getDerivedPosition())) {
        continue;
      }

      try {
        Evaluator->ProcessAttackEvent(
            AttackEvent(Attack.Damage, &EntHealth.Health, DT));

        if (EntHealth.Health <= 0) {
          Rs.RenderQueue->Enqueue(DestroyNodeEvent(EntOgre.EntityNode));

          Registry.destroy(EntInRange);
        } else if (HealthBarComponent* EntHealthBar =
                       Registry.try_get<HealthBarComponent>(EntInRange)) {
          // Fill may still be null for a frame or two while the render
          // thread catches up to a just-spawned unit's CreateHealthBarEvent
          if (EntHealthBar->Fill != nullptr) {
            const int CurrentDecile = static_cast<int>(std::round(
                (EntHealth.Health / EntHealth.MaxHealth) * 10.f));
            if (CurrentDecile != EntHealthBar->LastSyncedDecile) {
              EntHealthBar->LastSyncedDecile = CurrentDecile;
              Rs.RenderQueue->Enqueue(UpdateHealthBarEvent(
                  EntHealthBar->Fill, CurrentDecile / 10.f));
            }
          }
        }
      }

      catch (std::exception& e) {
        ECSReporter->EnqueueError(
            ErrorDetail::CreateError(ErrorCode::ATTACK_LOGIC_FAILED));
      }
    }
  }
}
