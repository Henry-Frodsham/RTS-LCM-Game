// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "GenericButton.h"
#include "RenderSystem.h"

TEST_CASE("GenericButton - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(GenericButton(
      "test_button", "Test", std::vector<float>{0.1f, 0.1f}, nullptr,
      [](EventQueue&, float, float) {}, nullptr, 101));
}

TEST_CASE("GenericButton - ChangeVisibility does not throw") {
  RenderSystem::GetInstance().Init();
  GenericButton Obj("vis_button", "Test", std::vector<float>{0.1f, 0.1f},
                    nullptr, [](EventQueue&, float, float) {}, nullptr, 102);
  CHECK_NOTHROW(Obj.ChangeVisibility(false));
  CHECK_NOTHROW(Obj.ChangeVisibility(true));
}

TEST_CASE("GenericButton - MaintainScaling does not throw") {
  RenderSystem::GetInstance().Init();
  GenericButton Obj("scale_button", "Test", std::vector<float>{0.2f, 0.2f},
                    nullptr, [](EventQueue&, float, float) {}, nullptr, 103);
  CHECK_NOTHROW(Obj.MaintainScaling());
}
