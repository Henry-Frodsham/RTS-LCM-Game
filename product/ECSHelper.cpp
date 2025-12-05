// Copyright © 2025 Henry Frodsham
#include "ECSHelper.h"
ECSHelper::ECSHelper(entt::registry* Registry, ErrorReporter* Reporter)
    : RegistryToUse(Registry), ParentReporter(Reporter) {
  FactoryBus = new EventBus();
  FactoryQueue = new EventQueue(FactoryBus);

  FactoryBus->Subscribe<CreateEntityEvent>(
      std::bind(&ECSHelper::CreateAndAddEntity, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddOgreComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddOgreComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddMeshComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddMeshComponent, this, std::placeholders::_1));
}

void ECSHelper::CreateAndAddEntity(CreateEntityEvent Event) {
  *Event.CreatedEntity = RegistryToUse->create();
}

void ECSHelper::CreateAndAddOgreComponent(AddOgreComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Ogre::SceneNode* Node = Rs.CreateSceneNode(Event.NodeName);
  Node->setPosition(Event.InitialPosition);

  RegistryToUse->emplace<OgreComponent>(*Event.Entity, Node, Event.NodeName);
}

void ECSHelper::CreateAndAddMeshComponent(AddMeshComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Ogre::SceneNode* NodeToAttach = nullptr;
  try {
    OgreComponent EntOgreComp =
        RegistryToUse->get<OgreComponent>(*Event.Entity);
    NodeToAttach = EntOgreComp.EntityNode;
  } catch (std::exception& e) {
    ParentReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ECS_WRONG_CREATION_ORDER,
        "failed to create MeshComponent due to this entity not having a "
        "OgreComponent to attach to"));
    return;
  }

  Ogre::Entity* Ent = Rs.CreateEntity(Event.EntityName, Event.MeshName);

  // attach this entity to the prexisting scene node
  NodeToAttach->attachObject(Ent);
  RegistryToUse->emplace<MeshComponent>(*Event.Entity, Ent, Event.MeshName,
                                        Event.EntityName);
}
