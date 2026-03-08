// Copyright © 2025 Henry Frodsham
#pragma once
#include "GameAction.h"

struct ContextActionCommand {
  ActionContext Context;
  ContextActionCommand(ActionContext Cntx) : Context(Cntx){}
};
struct UseActionCommand {
  ActionContext Context;
  UseActionCommand(ActionContext Cntx) : Context(Cntx) {}
};
struct BackActionCommand {
  ActionContext Context;
  BackActionCommand(ActionContext Cntx) : Context(Cntx) {}
};

struct PressActionCommand {
  ActionContext Context;
  bool Released;
  PressActionCommand(ActionContext Cntx, bool Release)
      : Context(Cntx), Released(Release) {}
};