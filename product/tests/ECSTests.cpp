// Copyright (c) 2025 Henry Frodsham
#include <OGRE/Ogre.h>
#include <doctest/doctest.h>

#include "RenderSystem.h"
#include "WorldManager.h"

TEST_CASE("ECS - constructor") { CHECK_NOTHROW(ErrorReporter Obj()); }
TEST_CASE("ECS - basic entity creation") {
  WorldManager* WM = new WorldManager(false);
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();
  WM->WorldQueue->Enqueue(
      CreateBasicWorldEntityEvent("test", Ogre::Vector3(0.5f, 0.f, -5.f)));
  WM->update(0.1f);
  // no easy way to access the registry directly directly so query ogre directly
  // returns nullptr when the sceneNode doesnt exist
  RS.RenderQueue->Dispatch();
  Ogre::SceneNode* SN = RS.GetSceneNodeFromName("test");

  CHECK(SN != nullptr);
}

TEST_CASE("ECS - entity with mesh creation") {
  WorldManager* WM = new WorldManager(false);
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();
  WM->WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "test2", "Sphere.mesh", "test45345ent", Ogre::Vector3(0.5f, 0.f, -5.f)));

  WM->update(0.1f);
  // no easy way to access the registry directly directly so query ogre directly
  // returns nullptr when the sceneNode doesnt exist
  RS.RenderQueue->Dispatch();
  Ogre::SceneNode* SN = RS.GetSceneNodeFromName("test2");

  CHECK(SN != nullptr);
}
