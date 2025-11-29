//Copyright © 2025 Henry Frodsham
#pragma once
#include <entt/entt.hpp>
#include "EventQueue.h"
#include "EventBus.h"
#include "BaseComponents.h"
#include "ECSFactoryEvent.h"
#include "RenderSystem.h"

//creates ECS entities
//seperated from WorldManager to consider future implementation of multiple ECS registries
//this class doesnt wrap entt::registry itself or change its behaviour, it simply contains construction templates for common components
class EntityComponentFactory {
public:
	EntityComponentFactory(entt::registry* Registry, ErrorReporter* Reporter);
	EntityComponentFactory() = default;

	EventQueue* FactoryQueue;
private:
	EventBus* FactoryBus;

	ErrorReporter* ParentReporter;

	entt::registry* RegistryToUse;


	//pass the event as R value since other events rely on the value of "Entity"
	void CreateAndAddEntity(CreateEntityEvent Event);

	void CreateAndAddOgreComponent(AddOgreComponentEvent Event);

	void CreateAndAddMeshComponent(AddMeshComponentEvent Event);

};