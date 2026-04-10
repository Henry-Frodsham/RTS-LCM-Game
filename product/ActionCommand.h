// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "GameAction.h"

// show the interaction wheel command
struct ContextActionCommand {
  ActionContext Context;
  explicit ContextActionCommand(ActionContext Cntx) : Context(Cntx) {}
};
struct UseActionCommand {
  ActionContext Context;
  explicit UseActionCommand(ActionContext Cntx) : Context(Cntx) {}
};
struct BackActionCommand {
  ActionContext Context;
  explicit BackActionCommand(ActionContext Cntx) : Context(Cntx) {}
};

// mouse button or left trigger pressed
struct PressActionCommand {
  ActionContext Context;
  bool Released;
  explicit PressActionCommand(ActionContext Cntx, bool Release)
      : Context(Cntx), Released(Release) {}
};
