// Copyright © 2025 Henry Frodsham
#pragma once

//definitions for all matrixable interactions, these are by nature very simple (thus can be evaluated by a matrix)
struct AttackEvent {
  int AttackPower;
  int &DefendingEntityHP;
  AttackEvent(int Power, int &DefendingHP)
      : AttackPower(Power), DefendingEntityHP(DefendingHP){}
};
