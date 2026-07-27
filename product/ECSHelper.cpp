// Copyright (c) 2025 Henry Frodsham
#include "ECSHelper.h"

#include <string>

#include "RenderEvent.h"

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
  FactoryBus->Subscribe<AddRangeComponentEvent>(std::bind(
      &ECSHelper::CreateAndAddRangeComponent, this, std::placeholders::_1));
  FactoryBus->Subscribe<NotifyConsequentialEntityStateChange>(
      std::bind(&ECSHelper::IssueRangeRevalEvent, this, std::placeholders::_1));
  FactoryBus->Subscribe<EntitiesInRangeUpdateEvent>(std::bind(
      &ECSHelper::UpdateAndColludeEntityRanges, this, std::placeholders::_1));
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
  } catch (std::exception e) {
  }
}

void ECSHelper::CreateAndAddRangeComponent(AddRangeComponentEvent Event) {
  std::set<entt::entity> EmptyRangeContainer;
  auto& Component = RegistryToUse->emplace<RangeCacheComponent>(
      *Event.Entity, EmptyRangeContainer);
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
  } catch (std::exception e) {
  }
}

void ECSHelper::UpdateAndColludeEntityRanges(EntitiesInRangeUpdateEvent Event) {
  auto RangeView =
      RegistryToUse->view<HealthComponent, AttackComponent, OwnershipComponent,
                          OgreComponent, RangeCacheComponent>();

  std::vector<entt::entity> RelevantEntities;
  entt::entity EntityOfInterest = FindEntityFromSceneNode(Event.OriginalNode);
  RangeCacheComponent& RangeCompToUpdate =
      RegistryToUse->get<RangeCacheComponent>(EntityOfInterest);
  OwnershipComponent& EntityOfInterestOwnership =
      RegistryToUse->get<OwnershipComponent>(EntityOfInterest);

  for (auto Entity : RangeView) {
    auto& Health = RangeView.get<HealthComponent>(Entity);
    auto& Attack = RangeView.get<AttackComponent>(Entity);
    auto& Ownership = RangeView.get<OwnershipComponent>(Entity);
    auto& Ogre = RangeView.get<OgreComponent>(Entity);
    auto& Range = RangeView.get<RangeCacheComponent>(Entity);

    if (!Event.EntitiesInRange.contains(Ogre.EntityNode)) {
      continue;
    }

    if (Ownership.PlayerID != EntityOfInterestOwnership.PlayerID) {
      RelevantEntities.push_back(Entity);
      // allow the entity in range to also attack this entity, since the other
      // entity may not have updated
      Range.EntitiesInRange.insert(EntityOfInterest);
    }
  }
  RangeCompToUpdate.EntitiesInRange =
      std::set<entt::entity>(RelevantEntities.begin(), RelevantEntities.end());
}

void ECSHelper::IssueRangeRevalEvent(
    NotifyConsequentialEntityStateChange Notif) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(
      RevalEntityRangeEvent(Notif.Entity, FactoryQueue, 10.f));
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

entt::entity ECSHelper::FindEntityFromSceneNode(Ogre::SceneNode* Node) {
  auto View = RegistryToUse->view<OgreComponent>();
  for (auto Entity : View) {
    OgreComponent EntComp = View.get<OgreComponent>(Entity);

    if (EntComp.EntityNode == Node) {
      return Entity;
    }
  }
  return entt::null;
}
