// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "EntityConstructionTemplates.h"
#include "Player.h"
#include "RenderSystem.h"
#include "WorldManager.h"

namespace {
// same world-space position for both entities so the attacker's arbitrary
// facing direction is always within the front-arc check (WorldManager's
// IsTargetWithinFrontArc auto-passes when the target is effectively on top
// of the attacker)
const Ogre::Vector3 kSharedPosition(0.5f, 0.f, -5.f);
const Ogre::Vector3f kSurfaceNormal(0.f, 1.f, 0.f);
}  // namespace

TEST_CASE("WorldManager - CreateAttackingGameObject provides combat "
         "components immediately, independent of ownership resolving") {
  WorldManager WM;
  Player Owner(1);

  auto Entity = EntityTemplates::CreateAttackingGameObject(
      WM.CompFactory,
      CreateAttackingEntityEvent("wm_combat_setup", "Sphere.mesh",
                                 "wm_combat_setup_ent", kSharedPosition,
                                 kSurfaceNormal, &Owner, {BiomeType::Plains},
                                 100.f, 10.f, 5.f, 1, 3.f));
  WM.update(0.f);

  HealthComponent* HC = WM.CompFactory->TryGetComponent<HealthComponent>(*Entity);
  MeleeAttackComponent* AC =
      WM.CompFactory->TryGetComponent<MeleeAttackComponent>(*Entity);
  RangeCacheComponent* RC =
      WM.CompFactory->TryGetComponent<RangeCacheComponent>(*Entity);

  REQUIRE(HC != nullptr);
  REQUIRE(AC != nullptr);
  REQUIRE(RC != nullptr);
  CHECK(HC->Health == 100.f);
  CHECK(AC->Damage == 5.f);
  CHECK(RC->Range == 10.f);
  CHECK(RC->EntitiesInRange.empty());
}

TEST_CASE("WorldManager - combat evaluation applies attacker damage to a "
         "defender placed in its range cache") {
  WorldManager WM;
  Player AttackerOwner(1);
  Player DefenderOwner(2);

  auto Attacker = EntityTemplates::CreateAttackingGameObject(
      WM.CompFactory,
      CreateAttackingEntityEvent("wm_attacker", "Sphere.mesh",
                                 "wm_attacker_ent", kSharedPosition,
                                 kSurfaceNormal, &AttackerOwner,
                                 {BiomeType::Plains}, 100.f, 10.f, 8.f, 1,
                                 3.f));
  auto Defender = EntityTemplates::CreateAttackingGameObject(
      WM.CompFactory,
      CreateAttackingEntityEvent("wm_defender", "Sphere.mesh",
                                 "wm_defender_ent", kSharedPosition,
                                 kSurfaceNormal, &DefenderOwner,
                                 {BiomeType::Plains}, 40.f, 10.f, 0.f, 2,
                                 3.f));
  WM.update(0.f);

  RangeCacheComponent* AttackerRange =
      WM.CompFactory->TryGetComponent<RangeCacheComponent>(*Attacker);
  REQUIRE(AttackerRange != nullptr);
  AttackerRange->EntitiesInRange.insert(*Defender);

  WM.update(0.5f);  // Damage(8) * DT(0.5) == 4

  HealthComponent* DefenderHealth =
      WM.CompFactory->TryGetComponent<HealthComponent>(*Defender);
  REQUIRE(DefenderHealth != nullptr);
  CHECK(DefenderHealth->Health == 36.f);
}

TEST_CASE("WorldManager - a defender destroyed by lethal damage is removed "
         "from the registry") {
  WorldManager WM;
  Player AttackerOwner(1);
  Player DefenderOwner(2);

  auto Attacker = EntityTemplates::CreateAttackingGameObject(
      WM.CompFactory,
      CreateAttackingEntityEvent("wm_lethal_attacker", "Sphere.mesh",
                                 "wm_lethal_attacker_ent", kSharedPosition,
                                 kSurfaceNormal, &AttackerOwner,
                                 {BiomeType::Plains}, 100.f, 10.f, 50.f, 1,
                                 3.f));
  auto Defender = EntityTemplates::CreateAttackingGameObject(
      WM.CompFactory,
      CreateAttackingEntityEvent("wm_lethal_defender", "Sphere.mesh",
                                 "wm_lethal_defender_ent", kSharedPosition,
                                 kSurfaceNormal, &DefenderOwner,
                                 {BiomeType::Plains}, 10.f, 10.f, 0.f, 2,
                                 3.f));
  WM.update(0.f);

  RangeCacheComponent* AttackerRange =
      WM.CompFactory->TryGetComponent<RangeCacheComponent>(*Attacker);
  REQUIRE(AttackerRange != nullptr);
  AttackerRange->EntitiesInRange.insert(*Defender);

  WM.update(1.f);  // Damage(50) * DT(1) far exceeds the defender's 10 HP

  HealthComponent* DefenderHealth =
      WM.CompFactory->TryGetComponent<HealthComponent>(*Defender);
  CHECK(DefenderHealth == nullptr);
}

TEST_CASE("WorldManager - unit production ticks progress into the owning "
         "player's queue once ownership resolves") {
  WorldManager WM;
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();

  Player Producer(1);

  EntityTemplates::CreateUnitProducingGameObject(
      WM.CompFactory,
      CreateUnitProducingGameObjectEvent(
          "wm_producer", "Sphere.mesh", "wm_producer_ent", kSharedPosition,
          kSurfaceNormal, &Producer, {BiomeType::Plains}, 60, 1));

  // let the ownership retry-chain converge: AddOwnerShipComponentEvent
  // requeues itself onto FactoryQueue until the entity's scene node (created
  // via RenderQueue) actually exists
  for (int i = 0; i < 6; ++i) {
    WM.update(0.f);
    RS.RenderQueue->Dispatch();
  }

  WM.update(0.5f);  // (60 unit/min / 60) * 100 * 0.5s == 50 progress
  Producer.Update();

  CHECK(Producer.UnitProdProg == 50.f);
}
