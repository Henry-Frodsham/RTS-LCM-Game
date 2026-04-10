// Copyright (c) 2025 Henry Frodsham
#include "ECSHelper.h"

#include "RenderEvent.h"

#include <string>

// helper class responsible for creating components
// however, doesnt manage or evaluate components
// (thats worldmanagers job)
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
  FactoryBus->Subscribe<AddOwnerShipComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddOwnerShipComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<MoveEntityAlongSphericalEvent>(std::bind(
      &ECSHelper::MoveEntityAlongSpherical, this, std::placeholders::_1));
  FactoryBus->Subscribe<OrientateEntityEvent>(std::bind(
      &ECSHelper::OrientateAndAdditionalSetup, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddUnitProductionEvent>(
      std::bind(&ECSHelper::CreateandAddUnitProductionComponent, this,
                std::placeholders::_1));
  FactoryBus->Subscribe<AddHealthEvent>(std::bind(
      &ECSHelper::CreateandAddHealthComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<AddAttackEvent>(std::bind(
      &ECSHelper::CreateandAddAttackComponent, this, std::placeholders::_1));
}

void ECSHelper::CreateAndAddEntity(CreateEntityEvent Event) {
  *Event.CreatedEntity = RegistryToUse->create();
}

void ECSHelper::CreateAndAddOgreComponent(AddOgreComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  auto& Component = RegistryToUse->emplace<OgreComponent>(
      *Event.Entity, nullptr, Event.NodeName);

  Rs.RenderQueue->Enqueue(
      CreateSceneNodeEvent(std::ref(Component.EntityNode), Event.NodeName));

  Rs.RenderQueue->Enqueue(SetNodePositionEvent(std::ref(Component.EntityNode),
                                               Event.InitialPosition));
}

void ECSHelper::CreateAndAddMeshComponent(AddMeshComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  Ogre::SceneNode* NodeToAttach = nullptr;
  try {
    OgreComponent& EntOgreComp =
        RegistryToUse->get<OgreComponent>(*Event.Entity);
    auto& Component = RegistryToUse->emplace<MeshComponent>(
        *Event.Entity, nullptr, Event.MeshName, Event.EntityName);

    Rs.RenderQueue->Enqueue(CreateOgreEntityEvent(
        std::ref(Component.Entity), Event.EntityName, Event.MeshName));

    Rs.RenderQueue->Enqueue(AttachEntityToScenNodeEvent(
        std::ref(Component.Entity), std::ref(EntOgreComp.EntityNode)));
  } catch (std::exception& e) {
    ParentReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ECS_WRONG_CREATION_ORDER,
        "failed to create MeshComponent due to this entity not having a "
        "OgreComponent to attach to"));
    return;
  }
}

void ECSHelper::CreateandAddUnitProductionComponent(
    AddUnitProductionEvent Event) {
  auto& Component = RegistryToUse->emplace<ProducesUnitsComponent>(
      *Event.Entity, Event.ProdPerM);
}
void ECSHelper::CreateandAddHealthComponent(AddHealthEvent Event) {
  auto& Component =
      RegistryToUse->emplace<HealthComponent>(*Event.Entity, Event.Health);
}
void ECSHelper::CreateandAddAttackComponent(AddAttackEvent Event) {
  auto& Component = RegistryToUse->emplace<AttackComponent>(
      *Event.Entity, Event.Radius, Event.Damage);
}
void ECSHelper::ChangeEntityVisibility(ChangeEntityVisibilityEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();

  try {
    OgreComponent Comp = FindEntityFromSceneNodeName(Event.NodeName);
    if (Comp.EntityNode != nullptr) {
      Comp.EntityNode->setVisible(Event.Visible);
    }
  } catch (std::exception& e) {
    ParentReporter->EnqueueError(ErrorDetail::CreateError(
        ErrorCode::ECS_WRONG_CREATION_ORDER,
        "failed to create MeshComponent due to this entity not having a "
        "OgreComponent to attach to"));
    return;
  }
}

void ECSHelper::MoveEntityAlongSpherical(MoveEntityAlongSphericalEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Ogre::Vector3 Pos = Event.Unit->getParentSceneNode()->getPosition();

  Ogre::Vector3 globeCenter = Ogre::Vector3(0.5f, 0.f, -5.f);
  float radius = (Pos - globeCenter).length();

  Ogre::Vector3 newDir = (Event.TargetPos - globeCenter).normalisedCopy();
  Ogre::Vector3 newPos = globeCenter + newDir * radius;

  Rs.RenderQueue->Enqueue(SetEntPositionEvent(Event.Unit, newPos));
}

void ECSHelper::CreateAndAddOwnerShipComponent(
    AddOwnerShipComponentEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  try {
    OgreComponent& EntOgreComp =
        RegistryToUse->get<OgreComponent>(*Event.Entity);

    if (RegistryToUse->try_get<OwnershipComponent>(*Event.Entity) == nullptr) {
      auto& Component = RegistryToUse->emplace<OwnershipComponent>(
          *Event.Entity, Event.PlayerID, Event.GamePlayer);
    }

    // ordering is very tricky for this, because when this runs the first time
    // the scenenode and entity havent been created good news is we can just
    // callback this and run it again until it works
    if (EntOgreComp.EntityNode != nullptr) {
      Rs.RenderQueue->Enqueue(
          AddOwnerShipToEntEvent(EntOgreComp.EntityNode, Event.PlayerID));
    } else {
      FactoryQueue->Enqueue(Event);
    }
  } catch (std::exception e) {}
}

void ECSHelper::OrientateAndAdditionalSetup(OrientateEntityEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  try {
    MeshComponent* EntOgreComp =
        RegistryToUse->try_get<MeshComponent>(*Event.Entity);

    if (EntOgreComp->Entity != nullptr) {
      Rs.RenderQueue->Enqueue(RotateEntToSurfaceNormalEvent(
          EntOgreComp->Entity, Ogre::Vector3(0.5f, 0.f, -5.f)));
    } else {
      FactoryQueue->Enqueue(Event);
    }
  } catch (std::exception e) {}
}
OgreComponent ECSHelper::FindEntityFromSceneNodeName(std::string NodeName) {
  auto View = RegistryToUse->view<OgreComponent>();
  for (auto Entity : View) {
    OgreComponent EntComp = View.get<OgreComponent>(Entity);

    if (EntComp.NodeName == NodeName) {
      return EntComp;
    }
  }
  return OgreComponent(nullptr, "");
}
