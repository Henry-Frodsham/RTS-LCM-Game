// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <entt/entt.hpp>
#include <set>

#include "CityComponents.h"
#include "CommonGameObjectComponents.h"
#include "Player.h"
#include "TraversalComponents.h"
#include "UnitComponents.h"

TEST_CASE("MovableComponent - only the explicitly allowed biomes are "
         "movable") {
  MovableComponent Comp({BiomeType::Plains, BiomeType::Forest}, 4.f);

  CHECK(Comp.MovableBiomes.at(BiomeType::Plains));
  CHECK(Comp.MovableBiomes.at(BiomeType::Forest));
  CHECK_FALSE(Comp.MovableBiomes.at(BiomeType::Ocean));
  CHECK_FALSE(Comp.MovableBiomes.at(BiomeType::Mountain));
  CHECK(Comp.Speed == 4.f);
}

TEST_CASE("MovableComponent - IsMoving reflects whether a path is queued") {
  MovableComponent Comp({BiomeType::Plains});
  CHECK_FALSE(Comp.IsMoving());

  Comp.Path.push_back(7);
  CHECK(Comp.IsMoving());

  Comp.Path.pop_front();
  CHECK_FALSE(Comp.IsMoving());
}

TEST_CASE("ExistableComponent - only the explicitly allowed biomes are "
         "existable") {
  ExistableComponent Comp({BiomeType::Desert});

  CHECK(Comp.ExistableBiomes.at(BiomeType::Desert));
  CHECK_FALSE(Comp.ExistableBiomes.at(BiomeType::Ocean));
  CHECK_FALSE(Comp.ExistableBiomes.at(BiomeType::Tundra));
}

TEST_CASE("HealthComponent - stores health and max health independently") {
  HealthComponent Comp(30.f, 100.f);
  CHECK(Comp.Health == 30.f);
  CHECK(Comp.MaxHealth == 100.f);
}

TEST_CASE("MeleeAttackComponent - stores damage") {
  MeleeAttackComponent Comp(12.5f);
  CHECK(Comp.Damage == 12.5f);
}

TEST_CASE("RangeCacheComponent - stores range and starts with the given "
         "entity set") {
  entt::entity Foe = static_cast<entt::entity>(1);
  std::set<entt::entity> Initial{Foe};

  RangeCacheComponent Comp(Initial, 8.f);

  CHECK(Comp.Range == 8.f);
  CHECK(Comp.EntitiesInRange.count(Foe) == 1);
  CHECK(Comp.EntitiesInRange.size() == 1);
}

TEST_CASE("FacingComponent - stores the initial forward vector") {
  Ogre::Vector3f Forward(0.f, 0.f, 1.f);
  FacingComponent Comp(Forward);
  CHECK(Comp.Forward == Forward);
}

TEST_CASE("OwnershipComponent - stores the player id and pointer") {
  Player Owner(9);
  OwnershipComponent Comp(9, &Owner);
  CHECK(Comp.PlayerID == 9);
  CHECK(Comp.GamePlayer == &Owner);
}
