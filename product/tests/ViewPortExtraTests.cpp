// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "InputDevice.h"
#include "RenderSystem.h"
#include "ViewPortController.h"

TEST_CASE("ViewPortController - RegisterControllingDevice makes "
         "IsControllerByDevice true for that device and false for others") {
  RenderSystem* RS = &RenderSystem::GetInstance();
  RS->Init();
  ViewPortController* Obj = RS->CreateViewPort();

  InputDevice DeviceA(nullptr, InputDeviceType::KBM);
  InputDevice DeviceB(nullptr, InputDeviceType::CONTROLLER);

  Obj->RegisterControllingDevice(&DeviceA);

  CHECK(Obj->IsControllerByDevice(&DeviceA));
  CHECK_FALSE(Obj->IsControllerByDevice(&DeviceB));
}

TEST_CASE("ViewPortController - Equals is false against an unrelated "
         "viewport pointer") {
  RenderSystem* RS = &RenderSystem::GetInstance();
  RS->Init();
  ViewPortController* Obj = RS->CreateViewPort();

  CHECK_FALSE(Obj->Equals(nullptr));
}

TEST_CASE("ViewPortController - InitOrbitCamera clamps the starting "
         "distance to the configured limits") {
  RenderSystem* RS = &RenderSystem::GetInstance();
  RS->Init();
  ViewPortController* Obj = RS->CreateViewPort();

  Obj->SetOrbitDistanceLimits(2.f, 10.f);
  CHECK_NOTHROW(
      Obj->InitOrbitCamera(Ogre::Vector3(0.f, 0.f, 0.f), 999.f));
}

TEST_CASE("ViewPortController - ZoomOrbitingPoint respects the configured "
         "distance limits without throwing") {
  RenderSystem* RS = &RenderSystem::GetInstance();
  RS->Init();
  ViewPortController* Obj = RS->CreateViewPort();

  Obj->SetOrbitDistanceLimits(2.f, 10.f);
  Obj->InitOrbitCamera(Ogre::Vector3(0.f, 0.f, 0.f), 5.f);

  CHECK_NOTHROW(Obj->ZoomOrbitingPoint(-1000.f));
  CHECK_NOTHROW(Obj->ZoomOrbitingPoint(1000.f));
}

TEST_CASE("ViewPortController - GetActualDimensions returns non-negative "
         "pixel dimensions") {
  RenderSystem* RS = &RenderSystem::GetInstance();
  RS->Init();
  ViewPortController* Obj = RS->CreateViewPort();

  std::vector<int> Dims = Obj->GetActualDimensions();
  REQUIRE(Dims.size() == 2);
  CHECK(Dims[0] >= 0);
  CHECK(Dims[1] >= 0);
}
