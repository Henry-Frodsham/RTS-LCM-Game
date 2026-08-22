// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Globe.h"
#include "Pathfinder.h"

namespace {

// BFS out to an exact hop distance - guarantees a tile that's genuinely N
// hops away rather than one that might loop back onto Start by chance
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

Globe MakeUniformGlobe(unsigned int Freq = 2) {
  Globe G;
  G.Generate(Freq, 1);
  for (uint32_t i = 0; i < G.GetTileCount(); ++i) {
    G.GetTile(i).SetBiome(BiomeType::Plains);
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

TEST_CASE("Pathfinder - BeginSearch seeds the open set with the start tile") {
  Globe G = MakeUniformGlobe();
  AStarState State = Pathfinder::BeginSearch(G, 0, G.GetTile(0).GetNeighbor(0));
  CHECK(State.Start == 0);
  CHECK_FALSE(State.OpenSet.empty());
  CHECK(State.GScore.at(0) == 0.f);
}

TEST_CASE("Pathfinder - a search where Start equals Goal succeeds "
         "immediately with an empty path") {
  Globe G = MakeUniformGlobe();
  AStarState State = Pathfinder::BeginSearch(G, 3, 3);
  const auto Movable = AllowOnly({BiomeType::Plains});
  const PathStatus Status = Pathfinder::Step(State, G, Movable, 64);

  CHECK(Status == PathStatus::Found);
  CHECK(Pathfinder::ReconstructPath(State).empty());
}

TEST_CASE("Pathfinder - reconstructed single-hop path lands on a real "
         "neighbor of Start") {
  Globe G = MakeUniformGlobe();
  const uint32_t Start = 0;
  const uint32_t Goal = G.GetTile(Start).GetNeighbor(0);

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const auto Movable = AllowOnly({BiomeType::Plains});
  RunToCompletion(State, G, Movable, 64);

  const std::deque<uint32_t> Path = Pathfinder::ReconstructPath(State);
  REQUIRE(Path.size() == 1);
  CHECK(Path.front() == Goal);
}

TEST_CASE("Pathfinder - fails immediately when no biome is movable") {
  Globe G = MakeUniformGlobe();
  const uint32_t Start = 0;
  const uint32_t Goal = G.GetTile(Start).GetNeighbor(0);

  AStarState State = Pathfinder::BeginSearch(G, Start, Goal);
  const std::unordered_map<BiomeType, bool> NothingMovable = AllowOnly({});
  const PathStatus Status = Pathfinder::Step(State, G, NothingMovable, 64);

  CHECK(Status == PathStatus::Failed);
}

TEST_CASE("Pathfinder - a wide per-step budget finds the same destination "
         "as a tight one") {
  Globe G = MakeUniformGlobe(3);
  const uint32_t Start = 0;
  const uint32_t Goal = FindTileAtHopDistance(G, Start, 3);
  REQUIRE(Goal != Start);
  const auto Movable = AllowOnly({BiomeType::Plains});

  AStarState TightState = Pathfinder::BeginSearch(G, Start, Goal);
  const PathStatus TightStatus = RunToCompletion(TightState, G, Movable, 1);

  AStarState WideState = Pathfinder::BeginSearch(G, Start, Goal);
  const PathStatus WideStatus = RunToCompletion(WideState, G, Movable, 1000);

  CHECK(TightStatus == PathStatus::Found);
  CHECK(WideStatus == PathStatus::Found);
  CHECK(Pathfinder::ReconstructPath(TightState).back() == Goal);
  CHECK(Pathfinder::ReconstructPath(WideState).back() == Goal);
}
