// Copyright © 2025 Henry Frodsham
#pragma once
#include <OGRE/Bites/OgreWindowEventUtilities.h>
#include <OGRE/Ogre.h>
#include <OGRE/Overlay/OgreImGuiOverlay.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <SDL2/SDL.h>
#include <Windows.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "ConfigManager.h"
#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "OverlayController.h"
#include "OverlayEvent.h"
#include "ViewPortController.h"
#include "RenderEvent.h"
#include "ViewPortUpdateListener.h"
#include "ViewPortUpdateEvent.h"
#include "RayTraceEvent.h"

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

  Ogre::RenderWindow* PrimaryWindow;

  ErrorReporter RenderErrorReporter;

  ViewPortController* DefaultViewPort;

  ViewPortUpdateListener* ViewPortListener;

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

  float GetDeltaTime();

  ViewPortController* FindViewPortFromDevice(InputDevice* Device);
};
