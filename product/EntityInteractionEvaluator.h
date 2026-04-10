// Copyright (c) 2025 Henry Frodsham
#pragma once
#include "EventBus.h"
#include "EventQueue.h"
#include "MatrixableInteractionEvent.h"
#include "TransformationMatrix.h"

// acts as a processor for entity interactions, processes interaction events and
// checks if they can be evaluated using a matrix
class EntityInteractionEvaluator {
 public:
  EntityInteractionEvaluator();

  void ProcessAttackEvent(AttackEvent Event);

  EventQueue* InteractionQueue;

 private:
  EventBus* InteractionBus;

  TransformationMatrix* AttackMatrix;
};
