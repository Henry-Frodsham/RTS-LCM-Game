#include <doctest/doctest.h>

#include "EntityInteractionEvaluator.h"
#include "MatrixableInteractionEvent.h"

TEST_CASE("basic matrixable interaction") {
  EntityInteractionEvaluator TestEvaluator = EntityInteractionEvaluator();
  int TestDefenderHP = 10;
  int TestAttackPower = 5;
  TestEvaluator.InteractionQueue->Enqueue(
      AttackEvent(TestAttackPower, &TestDefenderHP));
  TestEvaluator.InteractionQueue->Dispatch();
  CHECK(TestDefenderHP == 5);
}

TEST_CASE("complex non matrixable interaction") {

}
