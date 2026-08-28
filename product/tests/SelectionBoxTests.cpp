// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "RenderSystem.h"
#include "SelectionBox.h"

TEST_CASE("SelectionBox - construction does not throw") {
  RenderSystem::GetInstance().Init();
  CHECK_NOTHROW(SelectionBox("test_band", nullptr, "GREEN", 900));
}

TEST_CASE("SelectionBox - SetCorners accepts corners given in any order") {
  RenderSystem::GetInstance().Init();
  SelectionBox Band("order_band", nullptr, "GREEN", 901);

  // down and to the right
  CHECK_NOTHROW(Band.SetCorners(0.2f, 0.2f, 0.6f, 0.5f));
  // up and to the left, the same rectangle drawn backwards
  CHECK_NOTHROW(Band.SetCorners(0.6f, 0.5f, 0.2f, 0.2f));
  // a drag that never left its origin, so the box has no area at all
  CHECK_NOTHROW(Band.SetCorners(0.4f, 0.4f, 0.4f, 0.4f));
}

TEST_CASE("SelectionBox - visibility can be toggled repeatedly") {
  RenderSystem::GetInstance().Init();
  SelectionBox Band("visibility_band", nullptr, "GREEN", 902);

  CHECK_NOTHROW(Band.ChangeVisibility(true));
  // already visible, so this is the repeat a dragged cursor would cause
  CHECK_NOTHROW(Band.ChangeVisibility(true));
  CHECK_NOTHROW(Band.ChangeVisibility(false));
  CHECK_NOTHROW(Band.ChangeVisibility(false));
}

TEST_CASE("SelectionBox - a full drag of place, show, move and hide does not "
         "throw") {
  RenderSystem::GetInstance().Init();
  SelectionBox Band("drag_band", nullptr, "GREEN", 903);

  Band.SetCorners(0.1f, 0.1f, 0.15f, 0.15f);
  Band.ChangeVisibility(true);
  Band.SetCorners(0.1f, 0.1f, 0.4f, 0.35f);
  Band.SetCorners(0.1f, 0.1f, 0.8f, 0.9f);
  CHECK_NOTHROW(Band.ChangeVisibility(false));
  CHECK_NOTHROW(Band.MaintainScaling());
}
