// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericSlider.h"
#include "RenderSystem.h"

TEST_CASE("GenericSlider - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericSlider(
      "test_slider", "Test", std::vector<float>{0.1f, 0.1f},
      std::vector<float>{0.2f, 0.03f}, 1.f, 0.01f, nullptr,
      [](EventQueue&, float, float) {}, nullptr, 111));
}

TEST_CASE("GenericSlider - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("vis_slider", "Test", std::vector<float>{0.1f, 0.1f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, 0.01f, nullptr,
                    [](EventQueue&, float, float) {}, nullptr, 112);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericSlider - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("scale_slider", "Test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, 0.01f, nullptr,
                    [](EventQueue&, float, float) {}, nullptr, 113);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericSlider - repeated MaintainScaling calls after visibility "
         "toggles do not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("combo_slider", "Test", std::vector<float>{0.3f, 0.3f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, 0.01f, nullptr,
                    [](EventQueue&, float, float) {}, nullptr, 114);
  Obj.ChangeVisibility(false);
  CHECK_NOTHROW(Obj.MaintainScaling());
  Obj.ChangeVisibility(true);
  CHECK_NOTHROW(Obj.MaintainScaling());
}
