// Copyright © 2025 Henry Frodsham
#pragma once

// definitions for all matrixable interactions, these are by nature very simple
// (thus can be evaluated by a matrix)
struct AttackEvent {
  float AttackPower;
  float* DefendingEntityHP;
  float DeltaTime;
  AttackEvent(float Power, float* DefendingHP, float DT)
      : AttackPower(Power), DefendingEntityHP(DefendingHP), DeltaTime(DT) {}
};
