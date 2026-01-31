#include "EntityInteractionEvaluator.h"

EntityInteractionEvaluator::EntityInteractionEvaluator() {
	InteractionBus = new EventBus();
	InteractionQueue = new EventQueue(InteractionBus);
}