// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <vector>

// float health for units
struct HealthComponent {
  float Health;
  float MaxHealth;
  HealthComponent(float HP, float MaxHP) : Health(HP), MaxHealth(MaxHP) {}
};

// present only while a unit has been in a fight in the last few seconds -
// dealing damage as well as taking it, since a player watching their own unit
// swing needs to see which way it is pointing every bit as much as they need
// to see what is left of the thing it is hitting. removed again once
// Remaining runs out.
//
// its presence is one of the two things that put a unit's health bar and
// facing arrow on screen, the other being SelectedComponent. a unit nobody
// is looking at and nobody is fighting carries neither, and so draws nothing
struct InCombatComponent {
  float Remaining;
  explicit InCombatComponent(float Seconds) : Remaining(Seconds) {}
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
