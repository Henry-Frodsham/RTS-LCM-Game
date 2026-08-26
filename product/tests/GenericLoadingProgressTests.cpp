// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericLoadingProgress.h"
#include "RenderSystem.h"

TEST_CASE("GenericLoadingProgress - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericLoadingProgress(
      "test_bar", std::vector<float>{0.1f, 0.1f},
      std::vector<float>{0.3f, 0.03f}, nullptr, 121));
}

TEST_CASE("GenericLoadingProgress - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("vis_bar", std::vector<float>{0.1f, 0.1f},
                             std::vector<float>{0.3f, 0.03f}, nullptr, 122);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericLoadingProgress - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("scale_bar", std::vector<float>{0.2f, 0.2f},
                             std::vector<float>{0.3f, 0.03f}, nullptr, 123);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericLoadingProgress - SetProgress within [0,1] is kept") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("range_bar", std::vector<float>{0.2f, 0.2f},
                             std::vector<float>{0.3f, 0.03f}, nullptr, 124);
  Obj.SetProgress(0.f);
  CHECK(Obj.GetProgress() == doctest::Approx(0.f));

  Obj.SetProgress(0.5f);
  CHECK(Obj.GetProgress() == doctest::Approx(0.5f));

  Obj.SetProgress(1.f);
  CHECK(Obj.GetProgress() == doctest::Approx(1.f));
}

TEST_CASE("GenericLoadingProgress - SetProgress above 1 is clamped") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("over_bar", std::vector<float>{0.2f, 0.2f},
                             std::vector<float>{0.3f, 0.03f}, nullptr, 125);
  Obj.SetProgress(5.f);
  CHECK(Obj.GetProgress() == doctest::Approx(1.f));
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericLoadingProgress - SetProgress below 0 is clamped") {
  RenderSystem::GetInstance().Init();
  GenericLoadingProgress Obj("under_bar", std::vector<float>{0.2f, 0.2f},
                             std::vector<float>{0.3f, 0.03f}, nullptr, 126);
  Obj.SetProgress(-5.f);
  CHECK(Obj.GetProgress() == doctest::Approx(0.f));
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericLoadingProgress - undersized position and dimension vectors "
         "are rejected rather than indexed") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericLoadingProgress(
      "short_bar", std::vector<float>{}, std::vector<float>{0.3f}, nullptr,
      127));
}
