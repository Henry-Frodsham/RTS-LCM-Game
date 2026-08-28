// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <OgreMeshManager.h>
#include <OgrePrerequisites.h>

#include <atomic>
#include <future>  // NOLINT(build/c++11)

#include "ConfigManager.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "Globe.h"
#include "GlobeEvent.h"
#include "ErrorReporter.h"

// where a globe is in its build. a globe only exists once the game is being
// entered, so everything that reaches for one has to cope with there not being
// one yet - the accessors below answer with the defaults a globe would have
// rather than with a crash
enum class GlobeGenerationState { NotBuilt, Building, Built };

class GlobeInterface {
 public:
  GlobeInterface();

  void Initialise();

  void Update();

  // start building the globe, reading NumSubdivisions, CreationSeed and
  // PlanetRadius from GlobeSettings.json (see ConfigManager). the tile and
  // mesh work is handed to a worker thread and this returns immediately - the
  // finished data is picked up by Update, on the render thread, because only
  // that thread may hand a mesh to ogre
  void BeginGeneration(GenerateGlobeEvent Event);

  // how far through the build is, between 0 and 1. read from whichever thread
  // is drawing the progress bar, hence the atomic
  float GetGenerationProgress() const;

  GlobeGenerationState GetGenerationState() const;

  // GlobeSettings has been rewritten by an options page. the three values it
  // holds are only read when a globe is built, so this reloads the file and
  // stops there - the new numbers are used by the next BeginGeneration
  void ReloadConfiguration();

  void ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event);

  // the globe's own transform if there is a globe, and the transform one would
  // be built with if there is not, so a camera built before the world can
  // still be given sensible clip planes and orbit distances
  Ogre::Vector3f GetGlobeCentre() const;

  float GetGlobeRadius() const;

  GlobeRayHit CastRayFromWorld(const Ogre::Ray& WorldRay) const;

  // whether the planet itself stands between Viewer and Point. the horizon
  // test for a sphere: a point is visible from a viewer outside the sphere
  // exactly when it lies in front of the plane the viewer's horizon circle
  // sits in, which reduces to one dot product against the radius squared.
  // exact for a point on the surface, which is where every unit stands
  //
  // anything that works out what a camera can actually see needs this - a box
  // select drawn near the limb would otherwise reach through the planet, and
  // a screen space health bar would float over the horizon attached to a unit
  // nobody can see. answers false when there is no globe to be hidden behind
  bool IsPointBeyondHorizon(const Ogre::Vector3& Viewer,
                            const Ogre::Vector3& Point) const;

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

  ConfigManager* GlobeConfig;

  Ogre::SceneNode* GlobeSceneNode;
  Ogre::Entity* GlobeEntity;

  void InitialiseGlobeMaterials();

  // the globe the worker is filling in. it is only published as CGlobe once it
  // is complete, so nothing can be handed a half built world
  Globe* PendingGlobe;
  VisualMeshBufferData PendingMeshData;

  std::future<void> GenerationTask;
  std::atomic<float> GenerationProgress{0.f};
  std::atomic<GlobeGenerationState> GenerationState{
      GlobeGenerationState::NotBuilt};

  // hand the worker's output to ogre and put the globe on screen. only ever
  // called from Update, so only ever on the render thread
  void FinaliseGeneration();

  // whether the globe should be on screen, kept here rather than on the ogre
  // entity because the state that decides it can say so before there is an
  // entity to tell
  bool GlobeVisible;

  // the transform a globe is given when there is not one yet, so a camera
  // built at the menu is not built against zero
  static constexpr float DefaultRadius = 1.f;
};
