// Copyright © 2025 Henry Frodsham
#include "WorldManager.h"

WorldManager::WorldManager() {
  WorldBus = new EventBus();
  WorldQueue = new EventQueue(WorldBus);
  ECSReporter = new ErrorReporter();

  CompFactory = new ECSHelper(&Registry, ECSReporter);

  // dont subscribe to the CompFactory functions directly for initial entity
  // creation since it requires multiple steps so let templates handle that
  // subscribe using a lambda instead of std::bind since entity template
  // functions need the factory too
  WorldBus->Subscribe<CreateBasicWorldEntityEvent>(
      [this](const CreateBasicWorldEntityEvent& Event) {
        EntityTemplates::ConstructBasicEntity(CompFactory, Event);
      });

  WorldBus->Subscribe<CreateMeshWorldEntityEvent>(
      [this](const CreateMeshWorldEntityEvent& Event) {
        EntityTemplates::ConstructMeshEntity(CompFactory, Event);
      });

  WorldBus->Subscribe<ChangeEntityVisibilityEvent>([this](
      const ChangeEntityVisibilityEvent& Event) {
    CompFactory->ChangeEntityVisibility(Event);
  });
  WorldBus->Subscribe<ChangeGlobeVisibilityEvent>(std::bind(
      &WorldManager::ChangeGlobeVisibility, this, std::placeholders::_1));
  CreateGlobeMesh();
}

void WorldManager::CreateGlobeMesh() {
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "PlanetBase.mesh", "GlobeBase", Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "antartica_n.mesh", "GlobeAntarticaN",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "antartica_s.mesh", "GlobeAntarticaS",
      Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "europa.mesh", "GlobeEuropa",
      Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "greenland.mesh", "GlobeGreenland",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "iceland.mesh", "GlobeIceland",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(
      CreateMeshWorldEntityEvent("GlobeNode", "UK.mesh", "GlobeUK",
                                 Ogre::Vector3(0.5f, 0.f, -5.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "unit.mesh", "unit_test", Ogre::Vector3(0.5f, 0.f, -3.f)));
  WorldQueue->Enqueue(CreateMeshWorldEntityEvent(
      "GlobeNode", "city.mesh", "city_test", Ogre::Vector3(1.f, 0.f, -3.f)));
}

void WorldManager::ChangeGlobeVisibility(ChangeGlobeVisibilityEvent Event) {
  WorldQueue->Enqueue(ChangeEntityVisibilityEvent("GlobeNode", Event.Visible));
}
void WorldManager::update() {
  ECSReporter->Dispatch();
  WorldQueue->Dispatch();
  CompFactory->FactoryQueue->Dispatch();
}
