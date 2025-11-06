//Copyright © 2025 Henry Frodsham
#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include <entt/entt.hpp>

// direct owner of the ECS registry, all game objects on the game map are stored here however game logic is delegated
class WorldManager {
public:
	// any thread can queue events but cant directly publish to bus
	EventQueue WorldQueue;
private:
	EventBus WorldBus;

	// all of the world objects are stored here
	entt::registry Registry;
};