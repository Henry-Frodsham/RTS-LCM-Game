// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <entt/entt.hpp>

#include "EventQueue.h"
#include "InputTranslator.h"
#include "InteractionWheel.h"
#include "PlayerControlEvent.h"
#include "RayTraceEvent.h"
#include "RenderSystem.h"
#include "SelectionBox.h"
#include "ShareInfoEvent.h"
// class responsible for general player control over the game world
// such as (but not limited to), rotating the view point
// checking hit points
class PlayerGeneralControl {
 public:
  PlayerGeneralControl(InputTranslator* Translator, EventQueue* Queue,
                       InteractionWheel* Wheel);
  InputTranslator* PlayerTranslator;

  EventQueue* ControlQueue;
  InteractionWheel* InteractionWheelToNotify;
  void Update(float Dt);

  // the whole select gesture, from the verb going down to it coming back up.
  // a short one picks with a ray and a long one picks with a box, and this is
  // the only place that decides which - see DragSelectCommand
  void OnSelectDrag(DragSelectCommand Cmd);

  void OnCompletedTrace(RayPickResult Event);
  void OnCompletedBoxSelect(BoxPickResult Event);
  EventBus* TriggerBus;

 private:
  EventQueue* TriggerQueue;

  // the rubber band on screen while a box is being dragged. owned here rather
  // than by the interaction wheel because it belongs to the gesture, and the
  // gesture is this class's business - the wheel only ever hears the result
  SelectionBox* SelectionRectangle;

  // issues a fresh raytrace at the current cursor position, used every frame
  // while hold-to-preview is active - the select gesture only picks once, on
  // release, so it cant be the thing that keeps the preview fed
  void TriggerPreviewRayTrace();

  // the click half of a finished select gesture - one ray at where the cursor
  // came back up, which is either a unit or a piece of ground
  void TriggerSelectRayTrace(const ActionContext& Context, bool Additive);

  // the box half - the rectangle the gesture swept, handed to the render side
  // to be turned into the list of entities inside it
  void TriggerBoxSelect(const DragSelectCommand& Cmd);

  bool WasPreviewing = false;
};
