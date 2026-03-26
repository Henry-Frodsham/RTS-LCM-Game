#pragma once

struct UpdateUnitProgressEvent {
  float AddProg;
  UpdateUnitProgressEvent(float AP) : AddProg(AP) {}
};