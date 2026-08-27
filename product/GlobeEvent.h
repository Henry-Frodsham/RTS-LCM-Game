// Copyright (c) 2026 Henry Frodsham
#pragma once

struct ChangeGlobeVisibilityEvent {
  bool Visible;
  explicit ChangeGlobeVisibilityEvent(bool Vis) : Visible(Vis) {}
};

// build the world. the globe is not made at startup - the settings behind it
// are still being edited then - so it is asked for once, when the game is
// actually being entered, and the state that asks watches the progress rather
// than waiting on it
struct GenerateGlobeEvent {};

// the globe has finished building and everything that was sized against it -
// the cameras' clip planes and orbit distances - is holding measurements taken
// when there was no globe to measure
struct GlobeGeneratedEvent {};
