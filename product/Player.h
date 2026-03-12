// Copyright © 2025 Henry Frodsham
#pragma once
#include "Empire.h"

// stores and handles player information, unique to a game instance thread
class Player {
 public:
  Player(Empire* Emp);
  ~Player() {}

  Empire* PlayerEmpire;
 private:
  
};
