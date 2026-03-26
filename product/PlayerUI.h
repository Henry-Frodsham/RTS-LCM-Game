#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include "Player.h"
#include "RenderSystem.h"

class PlayerUI {
 public:
  PlayerUI(Player* player, int ThreadId);

  void Update();

  EventQueue* PlayerUIQueue;
 private:

  void SetUp();

  EventBus* PlayerUIBus;

  Player* GamePlayer;

  int KnownCities = 0;
  int KnownUnits = 0;
  int KnownAUnits = 0;
  int KnownACities = 0;
  int KnownProd = 0;
  float KnownProg = 0.f;
  int Id;
};