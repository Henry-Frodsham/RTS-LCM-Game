#pragma once
#include <entt/entt.hpp>

struct CreateEntityEvent {
	//reference counted ownership
	//extremely important for this case as the add component events rely on the value of CreatedEntity
	//so i need CreatedEntity to stick around in the heap until the other events are processed
	std::shared_ptr<entt::entity> CreatedEntity;

	CreateEntityEvent(std::shared_ptr<entt::entity> ent)
		: CreatedEntity(ent) { }
};

struct AddOgreComponentEvent {
	std::shared_ptr<entt::entity> Entity;
	std::string NodeName;
	AddOgreComponentEvent(std::shared_ptr<entt::entity> ent, std::string NodeN)
		: Entity(ent)
		, NodeName(NodeN){
	}
};

struct AddMeshComponentEvent {
	std::shared_ptr<entt::entity> Entity;
	std::string MeshName;
	std::string EntityName;

	AddMeshComponentEvent(std::shared_ptr<entt::entity> ent, std::string MeshN, std::string EntN)
		: Entity(ent)
		, MeshName(MeshN)
		, EntityName(EntN){
	}
};