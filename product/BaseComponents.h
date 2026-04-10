// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Ogre.h>

#include <string>

// common ECS components

// basic ogre information
// contains the attached scenenode for ogre
// contains no observable behaviour by itself
struct OgreComponent {
  Ogre::SceneNode* EntityNode;

  std::string NodeName;
  OgreComponent(Ogre::SceneNode* Node, std::string NodeN)
      : EntityNode(Node), NodeName(NodeN) {}
};

// mesh information
// contains the ogre entity with the stored name of the entity and mesh
// the mesh name is a file name for the .mesh file
// however due to the existence of ogre resource groups it doesnt need to be a
// filesystem::path
struct MeshComponent {
  Ogre::Entity* Entity;

  std::string MeshName;
  std::string EntityName;

  MeshComponent(Ogre::Entity* Ent, std::string MeshN, std::string EntN)
      : Entity(Ent), MeshName(MeshN), EntityName(EntN) {}
};
