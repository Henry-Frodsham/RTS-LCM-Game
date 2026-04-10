// Copyright (c) 2025 Henry Frodsham
#pragma once

// forward event to update progress to a placeable unit
struct UpdateUnitProgressEvent {
  float AddProg;
  explicit UpdateUnitProgressEvent(float AP) : AddProg(AP) {}
};
