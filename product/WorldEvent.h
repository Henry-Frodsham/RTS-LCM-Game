//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <entt/entt.hpp>

// full initial entity creation
struct CreateBasicWorldEntityEvent {
	//the name of the ogre::sceneNode this entity will be attached to / create
	std::string NodeName;
	CreateBasicWorldEntityEvent(std::string NodeN) : NodeName(NodeN) {}
};

struct CreateMeshWorldEntityEvent {
	std::string NodeName;
	std::string MeshName;
	std::string EntityName;
	CreateMeshWorldEntityEvent(std::string NodeN, std::string MeshN, std::string EntN)
		: NodeName(NodeN) 
		, MeshName(MeshN)
		, EntityName(EntN){}
};

//edit already created entities