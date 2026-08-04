// Copyright (c) 2026 Henry Frodsham
#pragma once

#include <OgreVector3.h>

#include <array>
#include <cstdint>

#include "Biome.h"

constexpr uint32_t InvalidTileID = UINT32_MAX;
constexpr uint32_t InvalidCountryID = UINT32_MAX;
constexpr uint32_t InvalidDetailID = UINT32_MAX;

// a singular tile contained within the globe
class Tile {
 public:
  Tile() = default;

  void SetCenter(const Ogre::Vector3& center) { Center = center; }
  const Ogre::Vector3& GetCenter() const { return Center; }

  void AddNeighbor(uint32_t tileID) {
    if (NeighborCount < Neighbors.size()) {
      Neighbors[NeighborCount++] = tileID;
    }
  }
  uint8_t GetNeighborCount() const { return NeighborCount; }
  uint32_t GetNeighbor(uint8_t index) const { return Neighbors[index]; }
  const std::array<uint32_t, 6>& GetNeighbors() const { return Neighbors; }

  void SetElevation(float elevation) { Elevation = elevation; }
  float GetElevation() const { return Elevation; }

  void SetBiome(BiomeType biome) { Biome = biome; }
  BiomeType GetBiome() const { return Biome; }
  bool IsLand() const { return Biome != BiomeType::Ocean; }

  void SetOwnerCountryID(uint32_t id) { OwnerCountryID = id; }
  uint32_t GetOwnerCountryID() const { return OwnerCountryID; }

  void SetLocalDetailID(uint32_t id) { LocalDetailID = id; }
  uint32_t GetLocalDetailID() const { return LocalDetailID; }

 private:
  Ogre::Vector3 Center = Ogre::Vector3::ZERO;  // unit-sphere direction

  std::array<uint32_t, 6> Neighbors{InvalidTileID, InvalidTileID,
                                    InvalidTileID, InvalidTileID,
                                    InvalidTileID, InvalidTileID};
  uint8_t NeighborCount = 0;

  float Elevation = 0.f;
  BiomeType Biome = BiomeType::Ocean;

  uint32_t OwnerCountryID = InvalidCountryID;
  uint32_t LocalDetailID =
      InvalidDetailID;  // reserved for future in-tile detail
};
