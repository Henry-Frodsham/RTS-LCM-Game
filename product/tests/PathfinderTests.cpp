// Copyright (c) 2026 Henry Frodsham

#include <doctest/doctest.h>

#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Globe.h"
#include "Pathfinder.h"

namespace {

// BFS out to an exact hop distance - unlike blindly following GetNeighbor(0)
// repeatedly, this can't loop back and land on Start by chance
uint32_t FindTileAtHopDistance(const Globe& G, uint32_t Start, int Hops) {
  std::vector<uint32_t> Frontier{Start};
  std::unordered_set<uint32_t> Visited{Start};
  for (int h = 0; h < Hops && !Frontier.empty(); ++h) {
    std::vector<uint32_t> NextFrontier;
    for (uint32_t T : Frontier) {
      const Tile& Tl = G.GetTile(T);
      for (uint8_t i = 0; i < Tl.GetNeighborCount(); ++i) {
        const uint32_t N = Tl.GetNeighbor(i);
        if (Visited.insert(N).second) {
          NextFrontier.push_back(N);
        }
      }
    }
    Frontier = std::move(NextFrontier);
  }
  return Frontier.empty() ? Start : Frontier.front();
}

// every tile forced to the same biome so tests can override specific tiles
// deterministically instead of depending on the noise-generated layout
Globe MakeUniformGlobe(unsigned int Freq = 2,
                       BiomeType Uniform = BiomeType::Plains) {
  Globe G;
  G.Generate(Freq, 1);
  for (uint32_t i = 0; i < G.GetTileCount(); ++i) {
    G.GetTile(i).SetBiome(Uniform);
  }
  return G;
}

std::unordered_map<BiomeType, bool> AllowOnly(
    std::initializer_list<BiomeType> Allowed) {
  std::unordered_map<BiomeType, bool> Map;
  for (int i = 0; i < static_cast<int>(BiomeType::Count); ++i) {
    Map[static_cast<BiomeType>(i)] = false;
  }
  for (BiomeType B : Allowed) {
    Map[B] = true;
  }
  return Map;
}

PathStatus RunToCompletion(AStarState& State, const Globe& G,
                           const std::unordered_map<BiomeType, bool>& Movable,
                           int BudgetPerStep, int MaxSteps = 10000) {
  PathStatus Status = PathStatus::InProgress;
  for (int i = 0; i < MaxSteps && Status == PathStatus::InProgress; ++i) {
    Status = Pathfinder::Step(State, G, Movable, BudgetPerStep);
  }
  return Status;
}

}  // namespace

TEST_CASE("Pathfinder - finds a path between adjacent tiles") {
  Globe G = MakeUniformGlobe();
  const uint32_t Start = 0;
  REQUIRE(G.GetTile(Start).GetNeighborCount() > 0);
  const uint32_t Goal = G.GetTile(Start).GetNeighbor(0);

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const auto Movable = AllowOnly({BiomeType::Plains});
  const PathStatus Status = RunToCompletion(State, G, Movable, 64);

  CHECK(Status == PathStatus::Found);
  const std::deque<uint32_t> Path = Pathfinder::ReconstructPath(State);
  REQUIRE(!Path.empty());
  CHECK(Path.back() == Goal);
}

TEST_CASE("Pathfinder - fails when the destination biome is impassable") {
  Globe G = MakeUniformGlobe();
  const uint32_t Start = 0;
  REQUIRE(G.GetTile(Start).GetNeighborCount() > 0);
  const uint32_t Goal = G.GetTile(Start).GetNeighbor(0);
  G.GetTile(Goal).SetBiome(BiomeType::Ocean);  // goal itself is impassable

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const auto Movable = AllowOnly({BiomeType::Plains});  // Ocean excluded
  const PathStatus Status = RunToCompletion(State, G, Movable, 64);

  CHECK(Status == PathStatus::Failed);
}

TEST_CASE(
    "Pathfinder - never routes through a tile whose biome is impassable") {
  Globe G = MakeUniformGlobe();
  const uint32_t Start = 0;
  const Tile& StartTile = G.GetTile(Start);
  REQUIRE(StartTile.GetNeighborCount() >= 2);

  const uint32_t Blocked = StartTile.GetNeighbor(0);
  const uint32_t Goal = StartTile.GetNeighbor(1);
  G.GetTile(Blocked).SetBiome(BiomeType::Ocean);

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const auto Movable = AllowOnly({BiomeType::Plains});
  const PathStatus Status = RunToCompletion(State, G, Movable, 64);

  CHECK(Status == PathStatus::Found);
  const std::deque<uint32_t> Path = Pathfinder::ReconstructPath(State);
  for (uint32_t TileID : Path) {
    CHECK(TileID != Blocked);
  }
}

TEST_CASE("Pathfinder - Step() resumes correctly across a tight per-call budget") {
  Globe G = MakeUniformGlobe(3);  // more tiles, forces a multi-hop search
  const uint32_t Start = 0;
  const uint32_t Goal = FindTileAtHopDistance(G, Start, 4);
  REQUIRE(Goal != Start);

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const auto Movable = AllowOnly({BiomeType::Plains});

  // one expansion per Step() call forces many resumptions across the search
  const PathStatus Status = RunToCompletion(State, G, Movable, 1);

  CHECK(Status == PathStatus::Found);
  const std::deque<uint32_t> Path = Pathfinder::ReconstructPath(State);
  REQUIRE(!Path.empty());
  CHECK(Path.back() == Goal);
}
