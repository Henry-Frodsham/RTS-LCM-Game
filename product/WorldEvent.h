//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <entt/entt.hpp>
#include <OGRE/Ogre.h>

// these events are for convenience only and dont provide an architectural benefit
// they are simply blueprints to create simple entities quickly

// full initial entity creation, without an attached mesh
// an entity of this type has no observable behaviour by itself
// creates an ogre::scenenode
struct CreateBasicWorldEntityEvent {
	//the name of the ogre::sceneNode this entity will be attached to / create
	std::string NodeName;
	Ogre::Vector3 InitialPosition;
	CreateBasicWorldEntityEvent(std::string NodeN, Ogre::Vector3 InitP) 
		: NodeName(NodeN)
		, InitialPosition(InitP){}
};
// full creation of an observable entity
// creates an ogre::entity with an attached mesh
// this event is an extension upon a BasicWorldEntity, so contains all behaviour from CreateBasicWorldEntityEvent
struct CreateMeshWorldEntityEvent {
	std::string NodeName;
	Ogre::Vector3 InitialPosition;
	std::string MeshName;
	std::string EntityName;
	CreateMeshWorldEntityEvent(std::string NodeN, std::string MeshN, std::string EntN, Ogre::Vector3 InitP)
		: NodeName(NodeN) 
		, MeshName(MeshN)
		, EntityName(EntN)
		, InitialPosition(InitP){}
};

//edit already created entities