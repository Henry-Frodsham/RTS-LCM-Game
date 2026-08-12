// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <OgreMeshManager.h>
#include <OgrePrerequisites.h>

#include "EventBus.h"
#include "EventQueue.h"
#include "Globe.h"
#include "GlobeEvent.h"
#include "ErrorReporter.h"

class GlobeInterface {
 public:
  GlobeInterface();

  void Initialise();

  void Update();

  void GenerateGlobe(GlobeCreationConfiguration Config);

  void ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event);

  Ogre::Vector3f GetGlobeCentre() { return CGlobe->GetCenter(); }

  float GetGlobeRadius() { return CGlobe->GetRadius(); }

  GlobeRayHit CastRayFromWorld(const Ogre::Ray& WorldRay) const;

  const Globe* GetGlobe() const { return CGlobe; }

  // world<->tile helpers for pathfinding/movement, mirroring the same
  // GlobeSceneNode-relative transform CastRayFromWorld already uses so a
  // path waypoint lands exactly where a click on that tile would
  uint32_t FindTileAtWorldPosition(const Ogre::Vector3& WorldPos) const;
  Ogre::Vector3f GetWorldPositionForTile(uint32_t TileID) const;
  Ogre::Vector3f GetWorldNormalForTile(uint32_t TileID) const;

 private:
  Globe* CGlobe;

  ErrorReporter* GlobeReporter;

  Ogre::SceneNode* GlobeSceneNode;
  Ogre::Entity* GlobeEntity;

  void InitialiseGlobeMaterials();

};
