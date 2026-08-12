// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <cstdint>
#include <deque>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Biome.h"
#include "Globe.h"

// incremental (budgeted) A* over a Globe's tile adjacency graph
// deliberately decoupled from any ECS component - callers hand in the
// traversable-biome map directly, so this stays pure graph/geometry logic
// that a component (see PathRequestComponent in TraversalComponents.h) can
// hold as resumable state across ticks

enum class PathStatus { InProgress, Found, Failed };

struct AStarNode {
  uint32_t TileID;
  float FScore;
};

struct AStarNodeCompare {
  // std::priority_queue is a max-heap by default, so invert the comparison
  // to get the lowest FScore out first
  bool operator()(const AStarNode& A, const AStarNode& B) const {
    return A.FScore > B.FScore;
  }
};

// resumable working memory for a single in-progress search
struct AStarState {
  uint32_t Start = InvalidTileID;
  uint32_t Goal = InvalidTileID;
  std::priority_queue<AStarNode, std::vector<AStarNode>, AStarNodeCompare>
      OpenSet;
  std::unordered_map<uint32_t, float> GScore;
  std::unordered_map<uint32_t, uint32_t> CameFrom;
  std::unordered_set<uint32_t> Closed;
};

namespace Pathfinder {

// node expansions spent per Step() call - keeps a long search from stalling
// a single tick, at the cost of spreading it across several
constexpr int kMaxExpansionsPerTick = 64;

AStarState BeginSearch(const Globe& G, uint32_t Start, uint32_t Goal);

// expands at most MaxExpansions nodes from State's open set. Safe to call
// again on the same State next tick if it returns InProgress
PathStatus Step(AStarState& State, const Globe& G,
                const std::unordered_map<BiomeType, bool>& MovableBiomes,
                int MaxExpansions);

// only valid to call after Step() has returned PathStatus::Found
// returns the tile path from Start to Goal, exclusive of Start
std::deque<uint32_t> ReconstructPath(const AStarState& State);

}  // namespace Pathfinder
