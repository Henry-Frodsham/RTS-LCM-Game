#pragma once
#include "Player.h"
#include "GameInstance.h"

struct OwnershipComponent {
  Player* OwningPlayer;
  GameInstance* OwningInstance;
  OwnershipComponent(Player* Player, GameInstance* Instance)
      : OwningPlayer(Player), OwningInstance(Instance) {}
};
