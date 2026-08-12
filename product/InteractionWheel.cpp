// Copyright (c) 2025 Henry Frodsham
#include "InteractionWheel.h"

#include <memory>
#include <vector>

#include "EntityConstructionTemplates.h"
InteractionWheel::InteractionWheel(InputTranslator* Device, int ThreadNum,
                                   ECSHelper* Interactor, Player* Play) {
  DeviceState = Device;
  ThreadID = ThreadNum;
  Factory = Interactor;
  RenderSystem& RS = RenderSystem::GetInstance();
  ForeignNotifBus = new EventBus();
  ForeignNotifQueue = new EventQueue(ForeignNotifBus);
  SelectedEntity = entt::null;
  GamePlayer = Play;
  Position = Ogre::Vector3f();
  SurfaceNormal = Ogre::Vector3f();
  Device->ActionBus->Subscribe<ContextActionCommand>(std::bind(
      &InteractionWheel::OnContextActionCommand, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<NotifyEntityResult>(
      std::bind(&InteractionWheel::ShareInfoSelectedEntReceive, this,
                std::placeholders::_1));
  ForeignNotifBus->Subscribe<NotifyRayResult>(std::bind(
      &InteractionWheel::ReceiveRayResult, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<CommitPathPreviewEvent>(std::bind(
      &InteractionWheel::CommitPathPreview, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<SelectEntitySuccessEvent>(
      std::bind(&InteractionWheel::SucessfulEntitySelection, this,
                std::placeholders::_1));

  ForeignNotifBus->Subscribe<CallBackACommand>(std::bind(
      &InteractionWheel::CallBackButtonA, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<CallBackBCommand>(std::bind(
      &InteractionWheel::CallBackButtonB, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<CallBackCCommand>(std::bind(
      &InteractionWheel::CallBackButtonC, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<CallBackDCommand>(std::bind(
      &InteractionWheel::CallBackButtonD, this, std::placeholders::_1));
  // events processed in serial so chaining like this isnt careless
  RS.RenderQueue->Enqueue(CreateOverlayEvent(
      "UI_Overlay_" + std::to_string(ThreadID), DeviceState->ManagedDevice));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent({0.f, 0.f}, {0.03f, 0.03f},
                         "interaction_wheel_A" + std::to_string(ThreadID),
                         "RED", "UI_Overlay_" + std::to_string(ThreadID)));
  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent({0.f, 0.f}, {0.03f, 0.03f},
                         "interaction_wheel_B" + std::to_string(ThreadID),
                         "RED", "UI_Overlay_" + std::to_string(ThreadID)));
  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent({0.f, 0.f}, {0.03f, 0.03f},
                         "interaction_wheel_C" + std::to_string(ThreadID),
                         "RED", "UI_Overlay_" + std::to_string(ThreadID)));
  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent({0.f, 0.f}, {0.03f, 0.03f},
                         "interaction_wheel_D" + std::to_string(ThreadID),
                         "RED", "UI_Overlay_" + std::to_string(ThreadID)));
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_A" + std::to_string(ThreadID),
      "interaction_wheel_A_text_" + std::to_string(ThreadID),
      "CITY",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_B" + std::to_string(ThreadID),
      "interaction_wheel_B_text_" + std::to_string(ThreadID),
      "DES",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_C" + std::to_string(ThreadID),
      "interaction_wheel_C_text_" + std::to_string(ThreadID),
      "BOAT",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_D" + std::to_string(ThreadID),
      "interaction_wheel_D_text_" + std::to_string(ThreadID),
      "UNIT",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_A" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_A_text_" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B_text_" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C_text_" + std::to_string(ThreadID), false));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D_text_" + std::to_string(ThreadID), false));
  Visibility = false;
}

void InteractionWheel::UpdateAndWarmupContext() {
  ForeignNotifQueue->Dispatch();
}

void InteractionWheel::ShareInfoSelectedEntReceive(NotifyEntityResult Event) {
  // the friendly check ECSHelper::ValidateEntitySelection would otherwise do
  // is done here first, synchronously, so a click on an enemy (or ownerless)
  // entity can fall through to a deselect - ValidateEntitySelection has no
  // failure path back to us, only a success one
  OwnershipComponent* OComp =
      Factory->TryGetComponent<OwnershipComponent>(Event.Entity);
  if (!OComp || OComp->GamePlayer != GamePlayer) {
    DeselectCurrent();
    return;
  }

  Factory->FactoryQueue->Enqueue(TrySelectEntityEvent(
      Event.Entity, GamePlayer, [Queue = ForeignNotifQueue](entt::entity Ent) {
        Queue->Enqueue(SelectEntitySuccessEvent(Ent));
      }));
}
void InteractionWheel::SucessfulEntitySelection(
    SelectEntitySuccessEvent Event) {
  if (SelectedEntity != entt::null && SelectedEntity != Event.Entity) {
    Factory->FactoryQueue->Enqueue(TryUnselectEntityEvent(SelectedEntity));
  }
  SelectedEntity = Event.Entity;
}

void InteractionWheel::DeselectCurrent() {
  if (SelectedEntity == entt::null) {
    return;
  }
  Factory->FactoryQueue->Enqueue(TryUnselectEntityEvent(SelectedEntity));
  SelectedEntity = entt::null;
}

void InteractionWheel::ReceiveRayResult(NotifyRayResult Event) {
  Position = Event.Pos;
  SurfaceNormal = Event.SurfaceNormal;
  SelectedBiome = Event.Biome;

  if (!PreviewActive) {
    // not mid hold-to-preview drag, so this is a genuine click (LMB / right
    // trigger) landing on empty ground rather than a drag-hover tick -
    // same "didn't click a friendly unit" case as ShareInfoSelectedEntReceive
    DeselectCurrent();
    return;
  }

  if (SelectedEntity == entt::null) {
    return;
  }

  RenderSystem& RS = RenderSystem::GetInstance();
  const uint32_t HoveredTile =
      RS.GetGlobeInterface()->FindTileAtWorldPosition(Event.Pos);
  if (HoveredTile == LastPreviewedTile) {
    return;
  }
  LastPreviewedTile = HoveredTile;

  Factory->FactoryQueue->Enqueue(RequestPathPreviewEvent(
      SelectedEntity, Position, SelectedBiome, GamePlayer));
}

void InteractionWheel::SetPreviewActive(bool Active) {
  if (PreviewActive == Active) {
    return;
  }
  PreviewActive = Active;
  if (!Active) {
    LastPreviewedTile = InvalidTileID;
    RenderSystem::GetInstance().RenderQueue->Enqueue(
        UpdatePathPreviewEvent({}, false));
  }
}

void InteractionWheel::CommitPathPreview(CommitPathPreviewEvent Event) {
  if (SelectedEntity != entt::null) {
    Factory->FactoryQueue->Enqueue(TryMoveEntityEvent(
        SelectedEntity, Position, SurfaceNormal, SelectedBiome, GamePlayer));
  }
}
void InteractionWheel::OnContextActionCommand(ContextActionCommand Cmd) {
  ActionContext Context = Cmd.Context;

  std::vector<float> Dimensions = DeviceState->GetViewPortDimensions();
  std::vector<float> SDimensions = DeviceState->GetScreenDimensions();
  Visibility = !Visibility;
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_A" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_A_text_" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B_text_" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C_text_" + std::to_string(ThreadID), Visibility));
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D_text_" + std::to_string(ThreadID), Visibility));

  float scaleX = SDimensions[0] / static_cast<float>(Dimensions[0]);
  float scaleY = SDimensions[1] / static_cast<float>(Dimensions[1]);

  RS.RenderQueue->Enqueue(
      OverlayEditPanelEvent("interaction_wheel_A" + std::to_string(ThreadID),
                            "UI_Overlay_" + std::to_string(ThreadID),
                            {0.03f, 0.03f}, {Context.MouseX, Context.MouseY}));
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_B" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {0.03f, 0.03f},
      {Context.MouseX + (0.03f * scaleX), Context.MouseY}));
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_C" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {0.03f, 0.03f},
      {Context.MouseX, Context.MouseY - 0.03f}));
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_D" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {0.03f, 0.03f},
      {Context.MouseX + (0.03f * scaleX), Context.MouseY - 0.03f}));

  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_A" + std::to_string(ThreadID),
      [](EventQueue& queue) { queue.Enqueue(CallBackACommand{}); },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B" + std::to_string(ThreadID),
      [](EventQueue& queue) { queue.Enqueue(CallBackBCommand{}); },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C" + std::to_string(ThreadID),
      [](EventQueue& queue) { queue.Enqueue(CallBackCCommand{}); },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D" + std::to_string(ThreadID),
      [](EventQueue& queue) { queue.Enqueue(CallBackDCommand{}); },
      ForeignNotifQueue));
}

void InteractionWheel::OnPressActionCommand(PressActionCommand Cmd) {
  ActionContext Context = Cmd.Context;
}

void InteractionWheel::CallBackButtonA(CallBackACommand Cmd) {
  // place
  if (!Position.isNaN()) {
    if (SelectedBiome == BiomeType::Ocean ||
        SelectedBiome == BiomeType::Mountain) {
      return;
    }
    if (GamePlayer->AvailableCities >= 1) {
      CityConstructionInfo Info = GamePlayer->PreCityPlace();

      std::shared_ptr<entt::entity> Ent =
          EntityTemplates::CreateUnitProducingGameObject(
              Factory, CreateUnitProducingGameObjectEvent(
                           Info.NodeName, "city.mesh", Info.EntName, Position,
                           SurfaceNormal, GamePlayer,
                           {BiomeType::Desert, BiomeType::Forest,
                            BiomeType::Plains, BiomeType::Tundra},
                           30, ThreadID));
      GamePlayer->AvailableCities -= 1;
    }
  }
}
void InteractionWheel::CallBackButtonB(CallBackBCommand Cmd) {
  // destroy
  if (SelectedEntity != entt::null) {
    Factory->FactoryQueue->Enqueue(TryDestroyEntityEvent(SelectedEntity));
  }
}
void InteractionWheel::CallBackButtonC(CallBackCCommand Cmd) {
  // boat
  if (!Position.isNaN()) {
    if (SelectedBiome != BiomeType::Ocean) {
      return;
    }
    if (GamePlayer->AvailableUnits >= 1) {
      UnitConstructionInfo Info = GamePlayer->PreUnitPlace();
      std::shared_ptr<entt::entity> Ent =
          EntityTemplates::CreateAttackingGameObject(
              Factory, CreateAttackingEntityEvent(
                           Info.NodeName, "unit.mesh", Info.EntName, Position,
                           SurfaceNormal, GamePlayer,
                           {BiomeType::Ocean},
                           100.f, 0.1f, 25.f, ThreadID, 0.3f));
      GamePlayer->AvailableUnits -= 1;
    }
  }
}
void InteractionWheel::CallBackButtonD(CallBackDCommand Cmd) {
  // unit
  if (!Position.isNaN()) {
    if (SelectedBiome == BiomeType::Ocean ||
        SelectedBiome == BiomeType::Mountain) {
      return;
    }
    if (GamePlayer->AvailableUnits >= 1) {
      UnitConstructionInfo Info = GamePlayer->PreUnitPlace();
      std::shared_ptr<entt::entity> Ent =
          EntityTemplates::CreateAttackingGameObject(
              Factory, CreateAttackingEntityEvent(
                           Info.NodeName, "unit.mesh", Info.EntName, Position,
                           SurfaceNormal, GamePlayer,
                           {BiomeType::Desert, BiomeType::Forest,
                            BiomeType::Plains, BiomeType::Tundra},
                           100.f, 0.1f, 25.f, ThreadID, 0.3f));
      GamePlayer->AvailableUnits -= 1;
    }
  }
}
