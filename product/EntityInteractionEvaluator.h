#pragma once
#include "EventQueue.h"
#include "EventBus.h"

// acts as a processor for entity interactions, processes interaction events and checks if they can be evaluated using a matrix
class EntityInteractionEvaluator {
public:
	EntityInteractionEvaluator();

	//the event must be typed to map to the correct event
	template <typename EventType>
	void ProcessEvent(EventType Event) {

	}

	EventQueue* InteractionQueue;
private:
	EventBus* InteractionBus;

};