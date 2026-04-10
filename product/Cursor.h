// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "CursorEvent.h"
#include "EventBus.h"
#include "InputTranslator.h"
#include "RenderSystem.h"

class Cursor {
 public:
  void ChangeVisibility(ChangeCursorVisibilityEvent Event);

  Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice,
         int CursorNumber);

  void Update();

 private:
  InputTranslator* DeviceState;

  EventBus* ForeignBus;

  int CursorID;
};
