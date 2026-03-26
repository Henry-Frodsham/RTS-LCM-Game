#pragma once
#include "Player.h"
struct OwnershipComponent {
  int PlayerID;
  Player* GamePlayer;
  OwnershipComponent(int Id, Player* P) : PlayerID(Id), GamePlayer(P) {}
};
