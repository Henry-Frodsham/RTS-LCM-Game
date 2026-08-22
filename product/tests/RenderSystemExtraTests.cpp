// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "RenderSystem.h"

TEST_CASE("RenderSystem - GetPrimaryViewport returns a usable viewport "
         "after Init") {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();
  CHECK(RS.GetPrimaryViewport() != nullptr);
}

TEST_CASE("RenderSystem - CreateViewPort returns a distinct viewport each "
         "call") {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();
  ViewPortController* First = RS.CreateViewPort();
  ViewPortController* Second = RS.CreateViewPort();
  CHECK(First != Second);
}

TEST_CASE("RenderSystem - GetRenderWindowDimensions returns positive width "
         "and height") {
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.Init();
  std::vector<float> Dims = RS.GetRenderWindowDimensions();
  REQUIRE(Dims.size() == 2);
  CHECK(Dims[0] > 0.f);
  CHECK(Dims[1] > 0.f);
}
