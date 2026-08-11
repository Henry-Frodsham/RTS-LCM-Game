// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <vector>

// float health for units
struct HealthComponent {
  float Health;
  float MaxHealth;
  HealthComponent(float HP, float MaxHP) : Health(HP), MaxHealth(MaxHP) {}
};

// attacker unit behaviour definer
struct MeleeAttackComponent {
  float Damage;
  MeleeAttackComponent(float D) : Damage(D) {}
};

// entities in range
struct RangeCacheComponent {
  std::set<entt::entity> EntitiesInRange;
  float Range;
  RangeCacheComponent(std::set<entt::entity> Entities, float R)
      : EntitiesInRange(Entities), Range(R) {}
};
