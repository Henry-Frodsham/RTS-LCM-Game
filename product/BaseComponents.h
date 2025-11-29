//Copyright © 2025 Henry Frodsham
#pragma once
#include <string>
#include <OGRE/Ogre.h>

// ogre entity information
struct OgreComponent {
	Ogre::SceneNode* EntityNode;

	std::string NodeName;
	OgreComponent(Ogre::SceneNode* Node, std::string NodeN)
		: EntityNode(Node)
		, NodeName(NodeN){ }
};

// mesh information, the 3d structure
struct MeshComponent {
	Ogre::Entity* Entity;

	std::string MeshName;
	std::string EntityName;

	MeshComponent(Ogre::Entity* Ent, std::string MeshN, std::string EntN)
		: Entity(Ent)
		, MeshName(MeshN)
		, EntityName(EntN){
	}
};

// material information

// texture information




