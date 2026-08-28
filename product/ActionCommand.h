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

// where a select gesture is in its life. the three edges a mouse drag has, so
// a listener can put a rubber band on screen (Begin/Update) and act on the
// rectangle it finished with (End), plus the case where the gesture is taken
// away rather than finished - a pad unplugged mid drag is never going to
// report the release that would have ended it. Cancel means clean up whatever
// was on screen and pick nothing
enum class DragPhase { Begin, Update, End, Cancel };

// the select verb (left mouse / right trigger) being pressed, dragged and
// released, reported as one gesture rather than as two unrelated press
// edges.
//
// a click and a box drag are the same gesture at different lengths, so
// theres deliberately no separate "click" command - a listener reads
// ExceededThreshold on the End phase to tell them apart. that also means the
// world is only picked once per click, on release, instead of once on each
// edge
struct DragSelectCommand {
  // the cursor now, in the normalised 0-1 viewport space every ActionContext
  // uses (see InputTranslator::MakeContext)
  ActionContext Context;

  // where the gesture started, same space
  float OriginX;
  float OriginY;

  DragPhase Phase;

  // the add-to-selection modifier was held when the gesture started - shift
  // on a keyboard, the unbound Y face button on a pad. latched at Begin so
  // letting go of shift mid drag cant change what the drag means
  bool Additive;

  // the cursor has travelled far enough from the origin for this to be a box
  // rather than a click. once true it stays true for the rest of the gesture
  bool ExceededThreshold;

  DragSelectCommand(ActionContext Cntx, float OX, float OY, DragPhase P,
                    bool Add, bool Exceeded)
      : Context(Cntx),
        OriginX(OX),
        OriginY(OY),
        Phase(P),
        Additive(Add),
        ExceededThreshold(Exceeded) {}
};
