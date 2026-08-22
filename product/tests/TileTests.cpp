// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "Tile.h"

TEST_CASE("Tile - default state") {
  Tile T;
  CHECK(T.GetCenter() == Ogre::Vector3::ZERO);
  CHECK(T.GetNeighborCount() == 0);
  CHECK(T.GetElevation() == 0.f);
  CHECK(T.GetBiome() == BiomeType::Ocean);
  CHECK_FALSE(T.IsLand());
  CHECK(T.GetOwnerCountryID() == InvalidCountryID);
  CHECK(T.GetLocalDetailID() == InvalidDetailID);
}

TEST_CASE("Tile - SetCenter and GetCenter round trip") {
  Tile T;
  Ogre::Vector3 Center(1.f, 2.f, 3.f);
  T.SetCenter(Center);
  CHECK(T.GetCenter() == Center);
}

TEST_CASE("Tile - AddNeighbor increments count and preserves insertion "
         "order") {
  Tile T;
  T.AddNeighbor(5);
  T.AddNeighbor(9);
  CHECK(T.GetNeighborCount() == 2);
  CHECK(T.GetNeighbor(0) == 5);
  CHECK(T.GetNeighbor(1) == 9);
}

TEST_CASE("Tile - AddNeighbor is capped at 6 neighbors") {
  Tile T;
  for (uint32_t i = 0; i < 10; ++i) {
    T.AddNeighbor(i);
  }
  CHECK(T.GetNeighborCount() == 6);
  CHECK(T.GetNeighbor(5) == 5);
}

TEST_CASE("Tile - GetNeighbors exposes the full backing array") {
  Tile T;
  T.AddNeighbor(42);
  const std::array<uint32_t, 6>& Neighbors = T.GetNeighbors();
  CHECK(Neighbors[0] == 42);
  CHECK(Neighbors[1] == InvalidTileID);
}

TEST_CASE("Tile - SetElevation and GetElevation round trip") {
  Tile T;
  T.SetElevation(0.75f);
  CHECK(T.GetElevation() == 0.75f);
}

TEST_CASE("Tile - Ocean biome keeps IsLand false") {
  Tile T;
  T.SetBiome(BiomeType::Ocean);
  CHECK_FALSE(T.IsLand());
}

TEST_CASE("Tile - a non-Ocean biome makes IsLand true") {
  Tile T;
  T.SetBiome(BiomeType::Mountain);
  CHECK(T.IsLand());
}

TEST_CASE("Tile - SetOwnerCountryID and GetOwnerCountryID round trip") {
  Tile T;
  T.SetOwnerCountryID(17);
  CHECK(T.GetOwnerCountryID() == 17);
}

TEST_CASE("Tile - SetLocalDetailID and GetLocalDetailID round trip") {
  Tile T;
  T.SetLocalDetailID(99);
  CHECK(T.GetLocalDetailID() == 99);
}
