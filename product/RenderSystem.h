// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <OGRE/Bites/OgreWindowEventUtilities.h>
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <Windows.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <iostream>
#include <string>
#include <vector>

#include "BillboardController.h"
#include "ConfigManager.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "GlobeInterface.h"
#include "HealthBarController.h"
#include "OverlayController.h"
#include "OverlayEvent.h"
#include "RayTraceEvent.h"
#include "RenderEvent.h"
#include "RenderQueryFlags.h"
#include "ViewPortController.h"
#include "ViewPortUpdateEvent.h"
#include "ViewPortUpdateListener.h"
#include "WorldEvent.h"

// base rendering system in singleton pattern
// responsible for attaching models to the main game map, and for creating
// viewports
class RenderSystem {
 private:
  RenderSystem();
  ~RenderSystem();

  bool IsInit = false;

  float DeltaTime;

  std::chrono::steady_clock::time_point LastFrameTime;

  Ogre::Root* OgreRoot;

  Ogre::SceneManager* SceneManager;

  Ogre::RaySceneQuery* RaySceneQuery;

  SDL_Window* SDLWindow;

  Ogre::OverlaySystem* OverlaySystem;

  OverlayController* OverlayControl;

  BillboardController* BillboardControl;

  HealthBarController* HealthBarControl;

  Ogre::RenderWindow* PrimaryWindow;

  ErrorReporter RenderErrorReporter;

  ViewPortController* DefaultViewPort;

  ViewPortUpdateListener* ViewPortListener;

  GlobeInterface* GlobeInt;

  // hold-to-preview path line, lazily created on first use. single shared
  // object rather than one per player - PATH_PREVIEW.material discards the
  // fragment per-viewport based on ownerID vs viewingPlayerID, the same
  // mechanism RED_UNIT/WHITE use for the selection highlight (see
  // AddOwnerShipToEnt), so one ManualObject is enough
  Ogre::ManualObject* PathPreviewLine;

  Ogre::RenderWindowDescription RenderWindowSettings;

  EventBus* RenderBus;

  ConfigManager* RenderConfig;

  // ogre has direct ownership of the viewport so cant use direct ownership
  std::vector<ViewPortController*> ViewPorts;

  void InitRenderResponsibility();

  void InitBasicResourceGroups();

  void UpdateExclusiveHandlers();

  void ScaleViewPorts();

  void CreateSceneNodeFromEvent(CreateSceneNodeEvent Event);

  void CreateEntityFromEvent(CreateOgreEntityEvent Event);

  void SetNodePositionFromEvent(SetNodePositionEvent Event);

  void SetEntPosFromEvent(SetEntPositionEvent Event);
  void AttachEntityToNodeFromEvent(AttachEntityToScenNodeEvent Event);

  void AssembleRayTraceEvent(StartRayTraceEvent Event);

  void ScaleEntityFromEvent(ScaleEntityEvent Event);

  void RotateEntityToSurfaceNormal(RotateEntToSurfaceNormalEvent Event);

  void ChangeEntityMaterial(ChangeEntMaterialEvent Event);

  void AddOwnerShipToEnt(AddOwnerShipToEntEvent Event);

  void EntityRangeCheck(RevalEntityRangeEvent Event);

  void CreateFacingArrow(CreateFacingArrowEvent Event);

  void UpdateFacingArrowOrientation(UpdateFacingArrowOrientationEvent Event);

  void ChangeFacingArrowVisibility(ChangeFacingArrowVisibilityEvent Event);

  void ChangeCameraOrbit(ChangeCameraOrbitAngleEvent Event);

  void ChangeCameraDepth(ChangeCameraOrbitDepthEvent Event);

  void DestroyNode(DestroyNodeEvent Event);
  void DestroyEntity(DestroyEntityEvent Event);

  void UpdatePathPreview(UpdatePathPreviewEvent Event);

 public:
  static RenderSystem& GetInstance();
  // RenderSystem(const RenderSystem&) = delete;
  // RenderSystem& operator=(const RenderSystem&) = delete;

  void Init();

  RenderSystem(const RenderSystem&) = delete;
  RenderSystem& operator=(const RenderSystem&) = delete;

  EventQueue* RenderQueue;

  ViewPortController* CreateViewPort();

  void RenderFrame();

  SDL_Window* GetSDLWindow();

  Ogre::RenderWindowDescription GetPrimaryWindowInformation();

  Ogre::SceneNode* CreateSceneNode(std::string Name);
  Ogre::SceneNode* GetSceneNodeFromName(std::string Name);

  Ogre::Entity* CreateEntity(std::string Name, std::string MeshName);

  ViewPortController* GetPrimaryViewport();

  GlobeInterface* GetGlobeInterface() const { return GlobeInt; }

  float GetDeltaTime();

  std::vector<float> GetRenderWindowDimensions();

  ViewPortController* FindViewPortFromDevice(InputDevice* Device);
};
