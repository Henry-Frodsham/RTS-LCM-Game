// Copyright © 2025 Henry Frodsham
#include "ECSHelper.h"

#include "RenderEvent.h"

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
  Ogre::Vector3 pos = Event.Unit->getPosition();

  Ogre::Quaternion lonRot(Ogre::Radian(Event.DeltaLatLon.y),
                          Ogre::Vector3::UNIT_Y);
  pos = lonRot * pos;

  Ogre::Vector3 latAxis =
      Ogre::Vector3::UNIT_Y.crossProduct(pos.normalisedCopy());
  if (latAxis.squaredLength() > 1e-6f) {
    latAxis.normalise();
    pos = Ogre::Quaternion(Ogre::Radian(Event.DeltaLatLon.x), latAxis) * pos;
  }

  Event.Unit->setPosition(pos.normalisedCopy() * Event.Radius);
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
