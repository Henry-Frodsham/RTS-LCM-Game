// Copyright (c) 2025 Henry Frodsham
#pragma once

// defines ticker behaviour where new units are produced
// stored in units per minute
struct ProducesUnitsComponent {
  int NumPerMinute;
  explicit ProducesUnitsComponent(int NPM) : NumPerMinute(NPM) {}
};
