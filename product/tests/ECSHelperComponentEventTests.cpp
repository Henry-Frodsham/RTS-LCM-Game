// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <entt/entt.hpp>
#include <memory>

#include "ECSHelper.h"
#include "ErrorReporter.h"

namespace {
std::shared_ptr<entt::entity> MakeEntity(ECSHelper& Factory) {
  auto Entity = std::make_shared<entt::entity>();
  Factory.FactoryQueue->Enqueue(CreateEntityEvent(Entity));
  return Entity;
}
}  // namespace

TEST_CASE("ECSHelper - AddHealthEvent creates a HealthComponent with Health "
         "equal to MaxHealth") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);

  auto Entity = MakeEntity(Factory);
  Factory.FactoryQueue->Enqueue(AddHealthEvent(Entity, 50.f));
  Factory.FactoryQueue->Dispatch();

  HealthComponent* HC = Factory.TryGetComponent<HealthComponent>(*Entity);
  REQUIRE(HC != nullptr);
  CHECK(HC->Health == 50.f);
  CHECK(HC->MaxHealth == 50.f);
}

TEST_CASE("ECSHelper - AddMeleeAttackEvent creates a MeleeAttackComponent "
         "with the given damage") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);

  auto Entity = MakeEntity(Factory);
  Factory.FactoryQueue->Enqueue(AddMeleeAttackEvent(Entity, 7.5f));
  Factory.FactoryQueue->Dispatch();

  MeleeAttackComponent* AC =
      Factory.TryGetComponent<MeleeAttackComponent>(*Entity);
  REQUIRE(AC != nullptr);
  CHECK(AC->Damage == 7.5f);
}

TEST_CASE("ECSHelper - AddRangeComponentEvent creates an empty "
         "RangeCacheComponent with the given range") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);

  auto Entity = MakeEntity(Factory);
  Factory.FactoryQueue->Enqueue(AddRangeComponentEvent(Entity, 12.f));
  Factory.FactoryQueue->Dispatch();

  RangeCacheComponent* RC =
      Factory.TryGetComponent<RangeCacheComponent>(*Entity);
  REQUIRE(RC != nullptr);
  CHECK(RC->Range == 12.f);
  CHECK(RC->EntitiesInRange.empty());
}

TEST_CASE("ECSHelper - AddExistableComponentEvent only allows the "
         "specified biomes") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);

  auto Entity = MakeEntity(Factory);
  Factory.FactoryQueue->Enqueue(AddExistableComponentEvent(
      Entity, {BiomeType::Plains, BiomeType::Forest}));
  Factory.FactoryQueue->Dispatch();

  ExistableComponent* EC =
      Factory.TryGetComponent<ExistableComponent>(*Entity);
  REQUIRE(EC != nullptr);
  CHECK(EC->ExistableBiomes.at(BiomeType::Plains));
  CHECK(EC->ExistableBiomes.at(BiomeType::Forest));
  CHECK_FALSE(EC->ExistableBiomes.at(BiomeType::Ocean));
}

TEST_CASE("ECSHelper - AddMovableComponentEvent stores speed and only "
         "allows the specified biomes") {
  entt::registry Registry;
  ErrorReporter Reporter;
  ECSHelper Factory(&Registry, &Reporter);

  auto Entity = MakeEntity(Factory);
  Factory.FactoryQueue->Enqueue(
      AddMovableComponentEvent(Entity, {BiomeType::Desert}, 6.f));
  Factory.FactoryQueue->Dispatch();

  MovableComponent* MC = Factory.TryGetComponent<MovableComponent>(*Entity);
  REQUIRE(MC != nullptr);
  CHECK(MC->Speed == 6.f);
  CHECK(MC->MovableBiomes.at(BiomeType::Desert));
  CHECK_FALSE(MC->MovableBiomes.at(BiomeType::Plains));
  CHECK_FALSE(MC->IsMoving());
}
