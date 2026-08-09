// Copyright (c) 2026 Henry Frodsham
#include "Pathfinder.h"

AStarState Pathfinder::BeginSearch(const Globe& G, uint32_t Start,
                                   uint32_t Goal) {
  AStarState State;
  State.Start = Start;
  State.Goal = Goal;

  if (Start == InvalidTileID || Goal == InvalidTileID) {
    return State;  // Step() reports Failed immediately for an unset search
  }

  State.GScore[Start] = 0.f;
  const Ogre::Vector3 StartPos = G.GetTileSurfacePosition(Start);
  const Ogre::Vector3 GoalPos = G.GetTileSurfacePosition(Goal);
  State.OpenSet.push(AStarNode{Start, (GoalPos - StartPos).length()});
  return State;
}

PathStatus Pathfinder::Step(
    AStarState& State, const Globe& G,
    const std::unordered_map<BiomeType, bool>& MovableBiomes,
    int MaxExpansions) {
  if (State.Start == InvalidTileID || State.Goal == InvalidTileID) {
    return PathStatus::Failed;
  }
  if (State.Start == State.Goal) {
    return PathStatus::Found;
  }

  const Ogre::Vector3 GoalPos = G.GetTileSurfacePosition(State.Goal);

  int Expansions = 0;
  while (Expansions < MaxExpansions) {
    if (State.OpenSet.empty()) {
      return PathStatus::Failed;
    }

    const AStarNode Current = State.OpenSet.top();
    State.OpenSet.pop();

    // entries go stale when a cheaper route to the same tile is found later;
    // rather than support a decrease-key we just push a new entry and skip
    // the old one here for free (doesn't count against the tick budget)
    if (State.Closed.contains(Current.TileID)) {
      continue;
    }
    State.Closed.insert(Current.TileID);
    ++Expansions;

    if (Current.TileID == State.Goal) {
      return PathStatus::Found;
    }

    const Tile& CurrentTile = G.GetTile(Current.TileID);
    const Ogre::Vector3 CurrentPos = G.GetTileSurfacePosition(Current.TileID);
    const float CurrentG = State.GScore.at(Current.TileID);

    for (uint8_t i = 0; i < CurrentTile.GetNeighborCount(); ++i) {
      const uint32_t NeighborID = CurrentTile.GetNeighbor(i);
      if (State.Closed.contains(NeighborID)) continue;

      const BiomeType NeighborBiome = G.GetTile(NeighborID).GetBiome();
      const auto BiomeIt = MovableBiomes.find(NeighborBiome);
      if (BiomeIt == MovableBiomes.end() || !BiomeIt->second) {
        continue;  // impassable biome for this mover
      }

      const Ogre::Vector3 NeighborPos = G.GetTileSurfacePosition(NeighborID);
      const float TentativeG = CurrentG + (NeighborPos - CurrentPos).length();

      const auto ExistingIt = State.GScore.find(NeighborID);
      if (ExistingIt != State.GScore.end() &&
          TentativeG >= ExistingIt->second) {
        continue;  // not an improvement over a route already known
      }

      State.GScore[NeighborID] = TentativeG;
      State.CameFrom[NeighborID] = Current.TileID;

      const float Heuristic = (GoalPos - NeighborPos).length();
      State.OpenSet.push(AStarNode{NeighborID, TentativeG + Heuristic});
    }
  }

  return PathStatus::InProgress;
}

std::deque<uint32_t> Pathfinder::ReconstructPath(const AStarState& State) {
  std::deque<uint32_t> Path;
  if (State.Start == InvalidTileID || State.Goal == InvalidTileID) {
    return Path;
  }

  uint32_t Current = State.Goal;
  while (Current != State.Start) {
    Path.push_front(Current);
    const auto It = State.CameFrom.find(Current);
    if (It == State.CameFrom.end()) {
      Path.clear();  // broken chain - shouldn't happen after a Found result
      break;
    }
    Current = It->second;
  }
  return Path;
}
