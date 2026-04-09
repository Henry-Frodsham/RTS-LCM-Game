// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <entt/entt.hpp>

#include "ECSHelper.h"
#include "EntityConstructionTemplates.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "WorldEvent.h"
#include "PlayerEvent.h"
#include "EntityInteractionEvaluator.h"
#include "UnitComponents.h"
#include "CityComponents.h"

// direct owner of the ECS registry, all game objects on the game map are stored
// here however game logic is delegated
class WorldManager {
 public:
  WorldManager(bool CreateGlobe = true);

  void update(float DT);

  // any thread can queue events but cant directly publish to bus
  EventQueue* WorldQueue;
  ECSHelper* CompFactory;

  void ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event);
 private:
  
  EventBus* WorldBus;

  ErrorReporter* ECSReporter;

  // all of the world objects are stored here
  entt::registry Registry;

  EntityInteractionEvaluator* Evaluator;
  void CreateGlobeMesh();

  void EvaluateTickerComponents(float DT);

  std::unordered_map<Ogre::SceneNode*, std::unordered_set<Ogre::SceneNode*>>
      CachedRangeEntities;

  void UpdateRangeCache(CachedEntitiesReturnEvent Event);
};
