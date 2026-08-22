// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "EntityInteractionEvaluator.h"
#include "MatrixableInteractionEvent.h"

TEST_CASE("EntityInteractionEvaluator - zero attack power leaves HP "
         "unchanged") {
  EntityInteractionEvaluator Evaluator;
  float HP = 10.f;
  Evaluator.InteractionQueue->Enqueue(AttackEvent(0.f, &HP, 1.f));
  Evaluator.InteractionQueue->Dispatch();
  CHECK(HP == 10.f);
}

TEST_CASE("EntityInteractionEvaluator - zero delta time leaves HP "
         "unchanged") {
  EntityInteractionEvaluator Evaluator;
  float HP = 10.f;
  Evaluator.InteractionQueue->Enqueue(AttackEvent(5.f, &HP, 0.f));
  Evaluator.InteractionQueue->Dispatch();
  CHECK(HP == 10.f);
}

TEST_CASE("EntityInteractionEvaluator - HP can go negative when attack "
         "power exceeds remaining HP") {
  EntityInteractionEvaluator Evaluator;
  float HP = 3.f;
  Evaluator.InteractionQueue->Enqueue(AttackEvent(10.f, &HP, 1.f));
  Evaluator.InteractionQueue->Dispatch();
  CHECK(HP == -7.f);
}

TEST_CASE("EntityInteractionEvaluator - multiple queued attacks apply "
         "cumulatively in a single dispatch") {
  EntityInteractionEvaluator Evaluator;
  float HP = 20.f;
  Evaluator.InteractionQueue->Enqueue(AttackEvent(4.f, &HP, 1.f));
  Evaluator.InteractionQueue->Enqueue(AttackEvent(6.f, &HP, 1.f));
  Evaluator.InteractionQueue->Dispatch();
  CHECK(HP == 10.f);
}
