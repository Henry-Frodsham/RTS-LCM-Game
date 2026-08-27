// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericSlider.h"
#include "RenderSystem.h"

TEST_CASE("GenericSlider - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericSlider(
      "test_slider", "test", std::vector<float>{0.1f, 0.1f},
      std::vector<float>{0.2f, 0.03f}, 0.f, 1.f, nullptr,
      [](EventQueue&, float) {}, nullptr, 111));
}

TEST_CASE("GenericSlider - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("vis_slider", "test", std::vector<float>{0.1f, 0.1f},
                    std::vector<float>{0.2f, 0.03f}, 0.f, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 112);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericSlider - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("scale_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.2f, 0.03f}, 0.f, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 113);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericSlider - repeated MaintainScaling calls after visibility "
         "toggles do not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("combo_slider", "test", std::vector<float>{0.3f, 0.3f},
                    std::vector<float>{0.2f, 0.03f}, 0.f, 1.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 114);
  Obj.ChangeVisibility(false);
  CHECK_NOTHROW(Obj.MaintainScaling());
  Obj.ChangeVisibility(true);
  CHECK_NOTHROW(Obj.MaintainScaling());
}

TEST_CASE("GenericSlider - SetValue maps onto the configured range") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("value_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 0.f, 100.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 115);

  CHECK(Obj.GetValue() == doctest::Approx(0.f));

  Obj.SetValue(50.f);
  CHECK(Obj.GetValue() == doctest::Approx(50.f));

  Obj.SetValue(100.f);
  CHECK(Obj.GetValue() == doctest::Approx(100.f));
}

TEST_CASE("GenericSlider - SetValue clamps outside the configured range") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("clamp_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 0.f, 100.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 116);

  Obj.SetValue(1000.f);
  CHECK(Obj.GetValue() == doctest::Approx(100.f));

  Obj.SetValue(-1000.f);
  CHECK(Obj.GetValue() == doctest::Approx(0.f));
}

TEST_CASE("GenericSlider - an empty range never divides by zero") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("zero_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 0.f, 0.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 117);

  CHECK_NOTHROW(Obj.SetValue(10.f));
  CHECK(Obj.GetValue() == doctest::Approx(0.f));
}

TEST_CASE("GenericSlider - a range that does not start at zero maps onto its "
         "own floor and ceiling") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("floor_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 640.f, 3840.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 119);

  // an untouched slider sits at the floor of its range, not at zero
  CHECK(Obj.GetValue() == doctest::Approx(640.f));

  Obj.SetValue(2240.f);
  CHECK(Obj.GetValue() == doctest::Approx(2240.f));

  // and the floor is a floor - it cannot be dragged or set below
  Obj.SetValue(0.f);
  CHECK(Obj.GetValue() == doctest::Approx(640.f));

  Obj.SetValue(10000.f);
  CHECK(Obj.GetValue() == doctest::Approx(3840.f));
}

TEST_CASE("GenericSlider - SetLabelText does not throw") {
  RenderSystem::GetInstance().Init();
  GenericSlider Obj("label_slider", "test", std::vector<float>{0.2f, 0.2f},
                    std::vector<float>{0.4f, 0.03f}, 0.f, 100.f, nullptr,
                    [](EventQueue&, float) {}, nullptr, 120);

  CHECK_NOTHROW(Obj.SetLabelText("test: 50"));
}

TEST_CASE("GenericSlider - undersized position and dimension vectors are "
         "rejected rather than indexed") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericSlider(
      "short_slider", "test", std::vector<float>{}, std::vector<float>{0.4f},
      0.f, 1.f, nullptr, [](EventQueue&, float) {}, nullptr, 118));
}
