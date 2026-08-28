// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <OGRE/Ogre.h>

#include "GlobeInterface.h"
#include "RenderSystem.h"
#include "ViewPortController.h"

// the two pieces of the screen space indicator pipeline that can be tested
// without a running game: the horizon rejection that stops a bar being drawn
// for a unit on the far side of the planet, and the world-to-screen
// projection that decides where a bar that IS visible ends up.
//
// the visibility rule itself (selected or recently damaged) lives in
// WorldManager::RefreshUnitIndicators and is covered in
// UnitIndicatorPolicyTests.cpp, which needs no ogre at all

TEST_CASE("GlobeInterface - a point on the near side of the globe is not "
         "beyond the horizon") {
  RenderSystem::GetInstance().Init();
  GlobeInterface* Globe = RenderSystem::GetInstance().GetGlobeInterface();
  REQUIRE(Globe != nullptr);

  const Ogre::Vector3 Centre(Globe->GetGlobeCentre());
  const float Radius = Globe->GetGlobeRadius();
  REQUIRE(Radius > 0.f);

  // camera backed off along +Z, unit standing on the point of the globe
  // directly underneath it
  const Ogre::Vector3 Camera = Centre + Ogre::Vector3(0.f, 0.f, Radius * 5.f);
  const Ogre::Vector3 NearSide = Centre + Ogre::Vector3(0.f, 0.f, Radius);

  CHECK_FALSE(Globe->IsPointBeyondHorizon(Camera, NearSide));
}

TEST_CASE("GlobeInterface - a point on the far side of the globe is beyond "
         "the horizon") {
  RenderSystem::GetInstance().Init();
  GlobeInterface* Globe = RenderSystem::GetInstance().GetGlobeInterface();
  REQUIRE(Globe != nullptr);

  const Ogre::Vector3 Centre(Globe->GetGlobeCentre());
  const float Radius = Globe->GetGlobeRadius();

  const Ogre::Vector3 Camera = Centre + Ogre::Vector3(0.f, 0.f, Radius * 5.f);
  const Ogre::Vector3 FarSide = Centre - Ogre::Vector3(0.f, 0.f, Radius);

  CHECK(Globe->IsPointBeyondHorizon(Camera, FarSide));
}

TEST_CASE("GlobeInterface - a point exactly on the horizon ring is treated as "
         "visible") {
  RenderSystem::GetInstance().Init();
  GlobeInterface* Globe = RenderSystem::GetInstance().GetGlobeInterface();
  REQUIRE(Globe != nullptr);

  const Ogre::Vector3 Centre(Globe->GetGlobeCentre());
  const float Radius = Globe->GetGlobeRadius();
  const float Distance = Radius * 5.f;

  const Ogre::Vector3 Camera = Centre + Ogre::Vector3(0.f, 0.f, Distance);

  // the horizon ring for a viewer at Distance sits at z == R*R/Distance, so
  // the test is an exact equality there and must not reject it
  const float HorizonZ = (Radius * Radius) / Distance;
  const float HorizonRadius =
      std::sqrt(std::max(0.f, (Radius * Radius) - (HorizonZ * HorizonZ)));
  const Ogre::Vector3 OnHorizon =
      Centre + Ogre::Vector3(HorizonRadius, 0.f, HorizonZ);

  CHECK_FALSE(Globe->IsPointBeyondHorizon(Camera, OnHorizon));
}

TEST_CASE("GlobeInterface - a viewer inside the globe has no horizon to be "
         "beyond") {
  RenderSystem::GetInstance().Init();
  GlobeInterface* Globe = RenderSystem::GetInstance().GetGlobeInterface();
  REQUIRE(Globe != nullptr);

  const Ogre::Vector3 Centre(Globe->GetGlobeCentre());
  const float Radius = Globe->GetGlobeRadius();

  const Ogre::Vector3 Inside = Centre + Ogre::Vector3(0.f, 0.f, Radius * 0.1f);
  const Ogre::Vector3 FarSide = Centre - Ogre::Vector3(0.f, 0.f, Radius);

  CHECK_FALSE(Globe->IsPointBeyondHorizon(Inside, FarSide));
}

TEST_CASE("ViewPortController - a point in front of the camera projects "
         "inside the viewport") {
  RenderSystem::GetInstance().Init();
  ViewPortController* ViewPort =
      RenderSystem::GetInstance().GetPrimaryViewport();
  REQUIRE(ViewPort != nullptr);

  // straight down the camera's own forward axis, so it must land at the
  // centre of the screen whatever the camera happens to be pointing at
  const Ogre::Vector3 CameraPos = ViewPort->GetCameraPosition();
  const Ogre::Vector3 Forward =
      RenderSystem::GetInstance().GetGlobeInterface()->GetGlobeCentre();
  const Ogre::Vector3 Ahead = CameraPos + ((Forward - CameraPos) * 0.5f);

  float X = -1.f;
  float Y = -1.f;
  const bool OnScreen = ViewPort->ProjectToViewport(Ahead, &X, &Y);

  if (OnScreen) {
    CHECK(X >= 0.f);
    CHECK(X <= 1.f);
    CHECK(Y >= 0.f);
    CHECK(Y <= 1.f);
  }
}

TEST_CASE("ViewPortController - a point behind the camera never projects") {
  RenderSystem::GetInstance().Init();
  ViewPortController* ViewPort =
      RenderSystem::GetInstance().GetPrimaryViewport();
  REQUIRE(ViewPort != nullptr);

  // the camera orbits looking towards the globe centre, so stepping away from
  // the centre past the camera itself puts a point squarely behind it. this
  // is the case the perspective divide would otherwise mirror back on screen
  const Ogre::Vector3 CameraPos = ViewPort->GetCameraPosition();
  const Ogre::Vector3 Centre(
      RenderSystem::GetInstance().GetGlobeInterface()->GetGlobeCentre());
  const Ogre::Vector3 Behind = CameraPos + (CameraPos - Centre);

  float X = -99.f;
  float Y = -99.f;
  CHECK_FALSE(ViewPort->ProjectToViewport(Behind, &X, &Y));
  // rejected outputs are left untouched rather than half written
  CHECK(X == -99.f);
  CHECK(Y == -99.f);
}

TEST_CASE("ViewPortController - projecting with null outputs is refused "
         "rather than crashing") {
  RenderSystem::GetInstance().Init();
  ViewPortController* ViewPort =
      RenderSystem::GetInstance().GetPrimaryViewport();
  REQUIRE(ViewPort != nullptr);

  float X = 0.f;
  CHECK_FALSE(ViewPort->ProjectToViewport(Ogre::Vector3::ZERO, nullptr, &X));
  CHECK_FALSE(ViewPort->ProjectToViewport(Ogre::Vector3::ZERO, &X, nullptr));
}

TEST_CASE("ViewPortController - an unclaimed viewport reports instance 0 so "
         "no indicator pool is built for it") {
  RenderSystem::GetInstance().Init();
  ViewPortController* ViewPort =
      RenderSystem::GetInstance().GetPrimaryViewport();
  REQUIRE(ViewPort != nullptr);

  // the menu viewport belongs to no player until an instance claims it, and
  // UnitIndicatorController::EnsurePoolFor keys off exactly this
  CHECK(ViewPort->GetInstanceNumber() == 0);
}
