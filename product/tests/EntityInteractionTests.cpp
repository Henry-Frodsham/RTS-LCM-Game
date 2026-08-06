// Copyright (c) 2025 Henry Frodsham
#include <doctest/doctest.h>

#include "EntityInteractionEvaluator.h"
#include "MatrixableInteractionEvent.h"

TEST_CASE("basic matrixable interaction") {
  EntityInteractionEvaluator TestEvaluator = EntityInteractionEvaluator();
  float TestDefenderHP = 10;
  float TestAttackPower = 5;
  TestEvaluator.InteractionQueue->Enqueue(
      AttackEvent(TestAttackPower, &TestDefenderHP, 1.f));
  TestEvaluator.InteractionQueue->Dispatch();
  CHECK(TestDefenderHP == 5); // NOLINT [readability/check]
}
