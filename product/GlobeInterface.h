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

 private:
  Globe* CGlobe;

  ErrorReporter* GlobeReporter;

  Ogre::SceneNode* GlobeSceneNode;
  Ogre::Entity* GlobeEntity;

  void InitialiseGlobeMaterials();

};
