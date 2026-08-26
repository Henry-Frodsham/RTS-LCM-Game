// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericSlider.h"
#include "RenderSystem.h"

TEST_CASE("GenericSlider - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericSlider(
      "test_slider", std::vector<float>{0.1f, 0.1f},
      std::vector<float>{0.2f, 0.03f}, 1.f, nullptr,
      [](EventQueue&, float) {}, nullptr, 111));
}

TEST_CASE("GenericSlider - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("vis_slider", std::vector<float>{0.1f, 0.1f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 112);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericSlider - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("scale_slider", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 113);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericSlider - repeated MaintainScaling calls after visibility "
         "toggles do not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("combo_slider", std::vector<float>{0.3f, 0.3f},
                    std::vector<float>{0.2f, 0.03f}, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 114);
  Obj.ChangeVisibility(false);
  CHECK_NOTHROW(Obj.MaintainScaling());
  Obj.ChangeVisibility(true);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericSlider - SetValue maps onto the configured range") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("value_slider", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 100.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 115);

  CHECK(Obj.GetValue() == doctest::Approx(0.f));

  Obj.SetValue(50.f);
  CHECK(Obj.GetValue() == doctest::Approx(50.f));

  Obj.SetValue(100.f);
  CHECK(Obj.GetValue() == doctest::Approx(100.f));
}

TEST_CASE("GenericSlider - SetValue clamps outside the configured range") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("clamp_slider", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 100.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 116);

  Obj.SetValue(1000.f);
  CHECK(Obj.GetValue() == doctest::Approx(100.f));

  Obj.SetValue(-1000.f);
  CHECK(Obj.GetValue() == doctest::Approx(0.f));
}

TEST_CASE("GenericSlider - a zero maximum never divides by zero") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("zero_slider", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 0.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 117);

  CHECK_NOTHROW(Obj.SetValue(10.f));
  CHECK(Obj.GetValue() == doctest::Approx(0.f));
}

TEST_CASE("GenericSlider - undersized position and dimension vectors are "
         "rejected rather than indexed") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericSlider(
      "short_slider", std::vector<float>{}, std::vector<float>{0.4f}, 1.f,
      nullptr, [](EventQueue&, float) {}, nullptr, 118));
}
