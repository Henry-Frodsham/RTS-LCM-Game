#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include <entt/entt.hpp>

class WorldManager {
public:
	// any thread can queue events but cant directly publish to bus
	EventQueue WorldQueue;
private:
	EventBus WorldBus;

	// all of the world objects are stored here
	entt::registry Registry;
};