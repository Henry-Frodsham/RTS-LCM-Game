// Copyright (c) 2025 Henry Frodsham
#include "EntityInteractionEvaluator.h"

EntityInteractionEvaluator::EntityInteractionEvaluator() {
  InteractionBus = new EventBus();
  InteractionQueue = new EventQueue(InteractionBus);

  // predefine attack matrix to reduce overhead
  Eigen::MatrixXf CombatMatrix(2, 2);
  CombatMatrix << 0.0f, 0.0f, -1.0f, 0.0f;

  AttackMatrix = new TransformationMatrix(CombatMatrix);

  InteractionBus->Subscribe<AttackEvent>(
      std::bind(&EntityInteractionEvaluator::ProcessAttackEvent, this,
                std::placeholders::_1));
}

// evaluate an attack event using the predefined matrix
void EntityInteractionEvaluator::ProcessAttackEvent(AttackEvent Event) {

  *Event.DefendingEntityHP -= Event.AttackPower * Event.DeltaTime;
}
