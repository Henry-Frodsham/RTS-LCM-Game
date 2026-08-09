// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <entt/entt.hpp>
#include <unordered_map>  // NOLINT(build/include_order)
#include <unordered_set>  // NOLINT(build/include_order)

#include "CityComponents.h"
#include "ECSHelper.h"
#include "EntityConstructionTemplates.h"
#include "EntityInteractionEvaluator.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "PlayerEvent.h"
#include "UnitComponents.h"
#include "WorldEvent.h"

// direct owner of the ECS registry, all game objects on the game map are stored
// here however game logic is delegated
class WorldManager {
 public:
  explicit WorldManager();

  void update(float DT);

  // any thread can queue events but cant directly publish to bus
  EventQueue* WorldQueue;
  ECSHelper* CompFactory;


 private:
  EventBus* WorldBus;

  ErrorReporter* ECSReporter;

  // all of the world objects are stored here
  entt::registry Registry;

  EntityInteractionEvaluator* Evaluator;


  void EvaluateTickerComponents(float DT);

  // advances any in-progress pathfinding searches by a bounded number of
  // node expansions per entity per tick (see Pathfinder::kMaxExpansionsPerTick)
  void AdvancePathRequests();

  // same budget-limited stepping as AdvancePathRequests, but for hold-to-
  // preview drags (MovableComponent::PreviewPath) - stays silent on
  // PathStatus::Failed since a preview missing its tiles is a constant,
  // expected occurrence rather than a reportable error
  void AdvancePathPreviewRequests();

  // steps entities with an active MovableComponent path towards their next
  // waypoint by Speed * DT, popping waypoints as they're reached
  void AdvanceMovingEntities(float DT);

  std::unordered_map<Ogre::SceneNode*, std::unordered_set<Ogre::SceneNode*>>
      CachedRangeEntities;

};
