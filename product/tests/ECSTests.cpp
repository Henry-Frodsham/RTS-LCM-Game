// Copyright © 2025 Henry Frodsham
#include <OGRE/Ogre.h>
#include <doctest/doctest.h>

#include "RenderSystem.h"
#include "WorldManager.h"
/*
TEST_CASE("basic entity creation") {
        WorldManager WM = WorldManager();
        RenderSystem& RS = RenderSystem::GetInstance();
        WM.WorldQueue->Enqueue(CreateBasicWorldEntityEvent("test",
Ogre::Vector3(0.5f, 0.f, -5.f))); WM.update();
        // no easy way to access the registry directly directly so query ogre
directly
        // returns nullptr when the sceneNode doesnt exist
        Ogre::SceneNode* SN = RS.GetSceneNodeFromName("test");

        CHECK(SN != nullptr);

}
*/
TEST_CASE("entity with mesh creation") {}
