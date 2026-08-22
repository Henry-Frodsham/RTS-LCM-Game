// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "ActionCommand.h"
#include "GameAction.h"

TEST_CASE("ActionContext - constructor stores all fields") {
  InputDevice Device(nullptr, InputDeviceType::KBM);
  ActionContext Ctx(0.25f, 0.75f, true, 3, &Device);

  CHECK(Ctx.MouseX == 0.25f);
  CHECK(Ctx.MouseY == 0.75f);
  CHECK(Ctx.JustPressed);
  CHECK(Ctx.ThreadId == 3);
  CHECK(Ctx.ActioningDevice == &Device);
}

TEST_CASE("PressActionCommand - stores its context and released flag") {
  InputDevice Device(nullptr, InputDeviceType::KBM);
  ActionContext Ctx(0.f, 0.f, false, 1, &Device);

  PressActionCommand Pressed(Ctx, false);
  PressActionCommand Released(Ctx, true);

  CHECK_FALSE(Pressed.Released);
  CHECK(Released.Released);
  CHECK(Pressed.Context.ThreadId == 1);
}

TEST_CASE("ContextActionCommand - stores the context it was built from") {
  InputDevice Device(nullptr, InputDeviceType::CONTROLLER);
  ActionContext Ctx(0.1f, 0.2f, true, 2, &Device);

  ContextActionCommand Cmd(Ctx);

  CHECK(Cmd.Context.MouseX == 0.1f);
  CHECK(Cmd.Context.ActioningDevice == &Device);
}

TEST_CASE("UseActionCommand and BackActionCommand store their context") {
  InputDevice Device(nullptr, InputDeviceType::KBM);
  ActionContext Ctx(0.5f, 0.6f, false, 4, &Device);

  UseActionCommand Use(Ctx);
  BackActionCommand Back(Ctx);

  CHECK(Use.Context.ThreadId == 4);
  CHECK(Back.Context.ThreadId == 4);
}
