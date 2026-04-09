#include "EntityInteractionEvaluator.h"

EntityInteractionEvaluator::EntityInteractionEvaluator() {
  InteractionBus = new EventBus();
  InteractionQueue = new EventQueue(InteractionBus);

  Eigen::MatrixXf CombatMatrix(2, 2);
  CombatMatrix << 0.0f, 0.0f, -1.0f, 0.0f;

  AttackMatrix = new TransformationMatrix(CombatMatrix);

  InteractionBus->Subscribe<AttackEvent>(
      std::bind(&EntityInteractionEvaluator::ProcessAttackEvent, this,
                std::placeholders::_1));
}
void EntityInteractionEvaluator::ProcessAttackEvent(AttackEvent Event) {
  Eigen::VectorXf Resource(2);
  Resource << (Event.AttackPower), (*Event.DefendingEntityHP);

  AttackMatrix->EvaluateAction(Resource, Event.DeltaTime);

  *Event.DefendingEntityHP = Resource(1);
}
