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

// there is deliberately no health bar component any more. a bar used to be a
// pair of billboards living in the scene next to the unit, which meant every
// unit carried render handles for a thing that is on screen perhaps one
// second in sixty. bars are drawn in screen space now, off a per-frame
// snapshot built by WorldManager::RefreshUnitIndicators, so a unit needs no
// per-unit render state for one at all - see UnitIndicatorController

// render handle for a unit's facing arrow. ArrowNode is a child of the
// unit's own SceneNode with inherited position but NOT inherited orientation
// (see RenderSystem::CreateFacingArrow) - it needs its own explicit world
// orientation, decoupled from the arbitrary twist RotateEntToSurfaceNormal
// leaves around the unit's local up axis.
//
// hidden by default. Shown mirrors what the render side was last told, so the
// per-frame visibility pass only enqueues an event when the answer actually
// changes rather than once a frame forever
struct FacingArrowComponent {
  Ogre::SceneNode* ArrowNode;
  Ogre::ManualObject* ArrowObject;
  bool Shown;

  FacingArrowComponent(Ogre::SceneNode* Node, Ogre::ManualObject* Obj)
      : ArrowNode(Node), ArrowObject(Obj), Shown(false) {}
};
