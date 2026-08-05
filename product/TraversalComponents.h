// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <unordered_map>

#include "Biome.h"

// store existable/movable biomes as a map, for fast checks on movement or
// placement queries

// entity has this if it cant move, sets restrictions on where it can move
struct MovableComponent {
  std::unordered_map<BiomeType, bool> MovableBiomes;
  explicit MovableComponent(const std::vector<BiomeType>& AllowedTypes) {
    for (int i = 0; i < static_cast<int>(BiomeType::Count); ++i) {
      MovableBiomes[static_cast<BiomeType>(i)] = false;
    }
    for (BiomeType Type : AllowedTypes) {
      MovableBiomes[Type] = true;
    }
  }
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