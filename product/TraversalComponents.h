// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <cstdint>
#include <deque>
#include <unordered_map>

#include "Biome.h"
#include "Pathfinder.h"

// store existable/movable biomes as a map, for fast checks on movement or
// placement queries

// entity has this if it cant move, sets restrictions on where it can move
// also owns the entity's active movement state - the remaining path is
// walked tile by tile in WorldManager::AdvanceMovingEntities
struct MovableComponent {
  std::unordered_map<BiomeType, bool> MovableBiomes;
  float Speed;                // world units/sec
  std::deque<uint32_t> Path;  // remaining tile IDs to visit, front() = next

  // path a hold-to-preview drag last resolved to, purely for display - never
  // touched by AdvanceMovingEntities, so a preview can never affect a real
  // move already in progress (or vice versa)
  std::deque<uint32_t> PreviewPath;

  explicit MovableComponent(const std::vector<BiomeType>& AllowedTypes,
                            float MoveSpeed = 3.f)
      : Speed(MoveSpeed) {
    for (int i = 0; i < static_cast<int>(BiomeType::Count); ++i) {
      MovableBiomes[static_cast<BiomeType>(i)] = false;
    }
    for (BiomeType Type : AllowedTypes) {
      MovableBiomes[Type] = true;
    }
  }

  bool IsMoving() const { return !Path.empty(); }
};

// current facing direction, a unit-length vector tangent to the globe
// surface at the entity's position. seeded from an arbitrary tangent of the
// spawn surface normal (a fresh unit has no movement history to derive a
// direction from) and updated from the movement delta each step of
// WorldManager::AdvanceMovingEntities - stays fixed while stationary so a
// unit keeps facing the way it last walked
struct FacingComponent {
  Ogre::Vector3f Forward;
  explicit FacingComponent(Ogre::Vector3f InitialForward)
      : Forward(InitialForward) {}
};

// transient - attached only while a path is being solved, removed once the
// search resolves (found or exhausted). see Pathfinder.h for the algorithm
struct PathRequestComponent {
  uint32_t GoalTile;
  AStarState Search;
  PathRequestComponent(uint32_t Goal, AStarState State)
      : GoalTile(Goal), Search(std::move(State)) {}
};

// same as PathRequestComponent but resolves into MovableComponent::PreviewPath
// instead of Path - kept as a distinct type so the WorldManager sweep driving
// it (AdvancePathPreviewRequests) can stay silent on PathStatus::Failed,
// unlike a real move request which reports PATH_DESTINATION_UNREACHABLE
struct PathPreviewRequestComponent {
  uint32_t GoalTile;
  AStarState Search;
  PathPreviewRequestComponent(uint32_t Goal, AStarState State)
      : GoalTile(Goal), Search(std::move(State)) {}
};

// entity has this to set restrictions on where it can exist
struct ExistableComponent {
  std::unordered_map<BiomeType, bool> ExistableBiomes;
  explicit ExistableComponent(const std::vector<BiomeType>& AllowedTypes) {
    for (int i = 0; i < static_cast<int>(BiomeType::Count); ++i) {
      ExistableBiomes[static_cast<BiomeType>(i)] = false;
    }
    for (BiomeType Type : AllowedTypes) {
      ExistableBiomes[Type] = true;
    }
  }
};