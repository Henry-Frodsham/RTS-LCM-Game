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

// how long a unit keeps showing its health bar and facing arrow after it
// was last in a fight. long enough to read a bar that is being chipped away
// and to see which way an attacker is pointing, short enough that a quiet map
// is a clean one
constexpr float kCombatIndicatorSeconds = 3.f;

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
  RefreshUnitIndicators(DT);
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
        } else {
          // nothing is pushed to the render side from here any more. landing
          // a hit only opens the window during which these two are worth
          // showing indicators for - what those indicators then say is read
          // straight off HealthComponent by RefreshUnitIndicators, so there
          // is no second copy of anybody's health to keep in step.
          //
          // both sides, not just the one being hit: the attacker's facing is
          // exactly what a player needs to see while it is swinging, since
          // facing is what IsTargetWithinFrontArc above gates the hit on
          Registry.emplace_or_replace<InCombatComponent>(
              EntInRange, kCombatIndicatorSeconds);
          Registry.emplace_or_replace<InCombatComponent>(
              Entity, kCombatIndicatorSeconds);
        }
      }

      catch (std::exception& e) {
        ECSReporter->EnqueueError(
            ErrorDetail::CreateError(ErrorCode::ATTACK_LOGIC_FAILED));
      }
    }
  }
}

void WorldManager::RefreshUnitIndicators(float DT) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  // age out the combat windows first, so a unit whose window closed this tick
  // is already gone by the time the snapshot below is built
  std::vector<entt::entity> Expired;
  auto CombatView = Registry.view<InCombatComponent>();
  for (auto [Entity, Combat] : CombatView.each()) {
    Combat.Remaining -= DT;
    if (Combat.Remaining <= 0.f) {
      Expired.push_back(Entity);
    }
  }
  for (entt::entity Entity : Expired) {
    Registry.remove<InCombatComponent>(Entity);
  }

  // health bars. the anchor is the top of the unit's mesh in world space,
  // which is where the bar wants to sit once it has been projected onto the
  // screen - measuring the mesh rather than guessing an offset is the one
  // thing the old billboard bar got right and is worth keeping
  std::vector<UnitIndicatorEntry> Snapshot;
  auto BarView = Registry.view<HealthComponent, OgreComponent, MeshComponent>();
  for (auto [Entity, Health, OgreComp, MeshComp] : BarView.each()) {
    if (!OgreComp.EntityNode || !MeshComp.Entity || Health.MaxHealth <= 0.f) {
      continue;
    }

    const SelectedComponent* Selected =
        Registry.try_get<SelectedComponent>(Entity);
    const bool Fighting = Registry.all_of<InCombatComponent>(Entity);
    if (!Selected && !Fighting) {
      continue;
    }

    // local rather than derived transforms: a unit's node is a direct child
    // of the root, so the two are the same thing, and the local pair is a
    // plain read where _getDerivedPosition can walk up the tree recomputing
    // cached state - not something to do from this thread while the render
    // thread is drawing. same reads AdvanceMovingEntities already makes
    const float UnitTop = MeshComp.Entity->getBoundingBox().getMaximum().y;
    const Ogre::Vector3 Anchor =
        OgreComp.EntityNode->getPosition() +
        (OgreComp.EntityNode->getOrientation() *
         Ogre::Vector3(0.f, UnitTop, 0.f));

    // a unit that is both selected and in a fight is public - a fight is not
    // a secret, and it would be odd for the same bar to be private to one
    // player only while nobody happens to be hitting it
    const int Exclusive = Fighting ? 0 : Selected->PlayerID;

    Snapshot.emplace_back(Anchor, Health.Health / Health.MaxHealth, Exclusive);
  }

  // sent unconditionally, empty included - an empty snapshot is how the last
  // bar left on screen gets taken off it
  Rs.RenderQueue->Enqueue(SyncUnitIndicatorsEvent(std::move(Snapshot)));

  // facing arrows. these are real scene nodes rather than pooled screen
  // space panels, so unlike the bars they are only touched on a change -
  // FacingArrowComponent::Shown is what the render side was last told
  auto ArrowView = Registry.view<FacingArrowComponent>();
  for (auto [Entity, Arrow] : ArrowView.each()) {
    if (!Arrow.ArrowNode) {
      continue;
    }

    const bool ShouldShow = Registry.all_of<SelectedComponent>(Entity) ||
                            Registry.all_of<InCombatComponent>(Entity);
    if (ShouldShow == Arrow.Shown) {
      continue;
    }

    Arrow.Shown = ShouldShow;
    Rs.RenderQueue->Enqueue(
        ChangeFacingArrowVisibilityEvent(Arrow.ArrowNode, ShouldShow));
  }
}
