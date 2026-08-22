// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericLoadingProgress.h"
#include "RenderSystem.h"

TEST_CASE("GenericLoadingProgress - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericLoadingProgress(
      "test_bar", "Loading", std::vector<float>{0.1f, 0.1f}, nullptr, 121));
}

TEST_CASE("GenericLoadingProgress - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("vis_bar", "Loading",
                             std::vector<float>{0.1f, 0.1f}, nullptr, 122);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericLoadingProgress - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("scale_bar", "Loading",
                             std::vector<float>{0.2f, 0.2f}, nullptr, 123);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericLoadingProgress - SetProgress within [0,1] does not throw") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("range_bar", "Loading",
                             std::vector<float>{0.2f, 0.2f}, nullptr, 124);
  CHECK_NOTHROW(Obj.SetProgress(0.f));
  CHECK_NOTHROW(Obj.SetProgress(0.5f));
  CHECK_NOTHROW(Obj.SetProgress(1.f));
}

TEST_CASE("GenericLoadingProgress - SetProgress above 1 is clamped without "
         "throwing") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("over_bar", "Loading",
                             std::vector<float>{0.2f, 0.2f}, nullptr, 125);
  CHECK_NOTHROW(Obj.SetProgress(5.f));
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericLoadingProgress - SetProgress below 0 is clamped without "
         "throwing") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("under_bar", "Loading",
                             std::vector<float>{0.2f, 0.2f}, nullptr, 126);
  CHECK_NOTHROW(Obj.SetProgress(-5.f));
  CHECK_NOTHROW(Obj.MaintainScaling());
}
