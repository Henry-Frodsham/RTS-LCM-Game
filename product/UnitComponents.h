// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <vector>

// float health for units
struct HealthComponent {
  float Health;
  explicit HealthComponent(float HP) : Health(HP) {}
};

// attacker unit behaviour definer
struct AttackComponent {
  float Radius;
  float Damage;
  AttackComponent(int R, int D) : Radius(R), Damage(D) {}
};

// entities in range
struct RangeCacheComponent {
  std::set<entt::entity> EntitiesInRange;
  RangeCacheComponent(std::set<entt::entity> Entities)
      : EntitiesInRange(Entities) {}
};
