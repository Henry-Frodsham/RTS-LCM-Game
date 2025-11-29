#pragma once
#include <string>
#include <array>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include <entt/entt.hpp>
#include "ECSFactoryEvent.h"
#include "WorldEvent.h"
#include "EntityComponentFactory.h"


namespace EntityTemplates {
	//construct a basic entity with an attached sceneNode
	inline std::shared_ptr<entt::entity> ConstructBasicEntity(
		EntityComponentFactory* Factory,
		CreateBasicWorldEntityEvent Event) {

		// garbage collector will destroy this when the function exits
		// hence why the entity is stored as a shared pointer
		auto Entity = std::make_shared<entt::entity>();

		Factory->FactoryQueue->Enqueue<CreateEntityEvent>(CreateEntityEvent(Entity));

		Factory->FactoryQueue->Enqueue<AddOgreComponentEvent>(AddOgreComponentEvent(Entity, Event.NodeName));

		// return the shared pointer for modular construction
		// wont hold a valid value until the factory is dispatched
		return Entity;
	}
	//construct an entity with an attached mesh (3d model)
	inline std::shared_ptr<entt::entity> ConstructMeshEntity(
		EntityComponentFactory* Factory,
		CreateMeshWorldEntityEvent Event) {

		auto Entity = ConstructBasicEntity(Factory, Event.NodeName);

		Factory->FactoryQueue->Enqueue<AddMeshComponentEvent>(AddMeshComponentEvent(Entity, Event.MeshName, Event.EntityName));

		return Entity;
	}
}