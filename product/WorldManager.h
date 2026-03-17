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

// direct owner of the ECS registry, all game objects on the game map are stored
// here however game logic is delegated
class WorldManager {
 public:
  WorldManager();

  void update();

  // any thread can queue events but cant directly publish to bus
  EventQueue* WorldQueue;

  void ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event);
 private:
  ECSHelper* CompFactory;

  EventBus* WorldBus;

  ErrorReporter* ECSReporter;

  // all of the world objects are stored here
  entt::registry Registry;

  void CreateGlobeMesh();
};
