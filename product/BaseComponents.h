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

// render handle for a unit's health bar billboard, plus render-sync
// throttle bookkeeping. HealthComponent::Health/MaxHealth stay the single
// source of truth - LastSyncedDecile just tracks the last health/maxHealth
// tenth the render side was told about, so combat doesn't enqueue a render
// event every tick
struct HealthBarComponent {
  Ogre::BillboardSet* BarSet;
  Ogre::Billboard* Fill;
  int LastSyncedDecile;

  HealthBarComponent(Ogre::BillboardSet* Set, Ogre::Billboard* F, int Decile)
      : BarSet(Set), Fill(F), LastSyncedDecile(Decile) {}
};

// render handle for a unit's facing-direction arrow. ArrowNode is a child of
// the unit's own SceneNode with inherited position but NOT inherited
// orientation (see RenderSystem::CreateFacingArrow) - it needs its own
// explicit world orientation, decoupled from the arbitrary twist
// RotateEntToSurfaceNormalEvent leaves around the unit's local up axis.
// Hidden by default, only shown while the unit is selected
struct FacingArrowComponent {
  Ogre::SceneNode* ArrowNode;
  Ogre::ManualObject* ArrowObject;

  FacingArrowComponent(Ogre::SceneNode* Node, Ogre::ManualObject* Obj)
      : ArrowNode(Node), ArrowObject(Obj) {}
};
