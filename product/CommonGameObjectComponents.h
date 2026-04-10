// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "Player.h"

// ownership tag with player id and a pointer to the player object
// used for checking who owns x thing
struct OwnershipComponent {
  int PlayerID;
  Player* GamePlayer;
  OwnershipComponent(int Id, Player* P) : PlayerID(Id), GamePlayer(P) {}
};
