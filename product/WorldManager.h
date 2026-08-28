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

  // decides which units should be showing a health bar and a facing arrow
  // this frame, and tells the render side.
  //
  // a unit shows its indicators while it is selected (SelectedComponent) or
  // has been in a fight in the last kCombatIndicatorSeconds
  // (InCombatComponent), and shows nothing the rest of the time. those
  // are the only two moments the information is worth the screen space, and
  // keeping the rule in one pass rather than spread across the places that
  // cause it is what stops a unit deselected mid fight losing the bar it
  // should still have.
  //
  // the bars go out as a single snapshot for the render side to pin to the
  // screen each frame; the arrows are per unit scene nodes, so those are
  // only touched when the answer for that unit actually changes
  void RefreshUnitIndicators(float DT);

  std::unordered_map<Ogre::SceneNode*, std::unordered_set<Ogre::SceneNode*>>
      CachedRangeEntities;

};
