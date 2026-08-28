// Copyright (c) 2025 Henry Frodsham
#include "InteractionWheel.h"

#include <memory>
#include <vector>

#include "EntityConstructionTemplates.h"
InteractionWheel::InteractionWheel(InputTranslator* Device, int ThreadNum,
                                   ECSHelper* Interactor, Player* Play)
    // the selection needs the factory to validate through and the player to
    // validate against, so it is built here rather than assigned below
    : Selection(Interactor, Play) {
  DeviceState = Device;
  ThreadID = ThreadNum;
  Factory = Interactor;
  RenderSystem& RS = RenderSystem::GetInstance();
  ForeignNotifBus = new EventBus();
  ForeignNotifQueue = new EventQueue(ForeignNotifBus);
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
  ForeignNotifBus->Subscribe<NotifyBoxSelectResult>(
      std::bind(&InteractionWheel::ReceiveBoxSelectResult, this,
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

// a click landed on an entity. what happens to the selection depends on whose
// entity it is and whether the add-to-selection modifier was held: somebody
// else's clears the set, one of yours joins it or replaces it, and one thats
// already in it is taken back out
void InteractionWheel::ShareInfoSelectedEntReceive(NotifyEntityResult Event) {
  Selection.PruneDestroyed();

  // the friendly check ECSHelper::ValidateEntitySelection would otherwise do
  // is done here first, synchronously, so a click on an enemy (or ownerless)
  // entity can fall through to a deselect - ValidateEntitySelection has no
  // failure path back to us, only a success one
  if (!Factory->IsValidEntity(Event.Entity)) {
    DeselectOnMiss(Event.Additive);
    return;
  }

  OwnershipComponent* OComp =
      Factory->TryGetComponent<OwnershipComponent>(Event.Entity);
  if (!OComp || OComp->GamePlayer != GamePlayer) {
    DeselectOnMiss(Event.Additive);
    return;
  }

  // shift-clicking something already selected takes it back out, which is the
  // only way to drop one unit out of a group without rebuilding the group
  if (Event.Additive && Selection.Contains(Event.Entity)) {
    Selection.Remove(Event.Entity);
    return;
  }

  if (!Event.Additive) {
    Selection.Clear();
  }
  Selection.Request(Event.Entity, ForeignNotifQueue);
}

// a finished rubber band. the entities inside it havent been checked for
// ownership yet - each goes through the same validation a clicked one does,
// and anything belonging to somebody else simply never answers
void InteractionWheel::ReceiveBoxSelectResult(NotifyBoxSelectResult Event) {
  Selection.PruneDestroyed();

  // an empty box over open ground is a deliberate "select nothing", so the
  // clear happens on the modifier alone and not on whether anything was found
  if (!Event.Additive) {
    Selection.Clear();
  }

  Selection.RequestMany(Event.Entities, ForeignNotifQueue);
}

void InteractionWheel::SucessfulEntitySelection(
    SelectEntitySuccessEvent Event) {
  Selection.Confirm(Event.Entity);
}

void InteractionWheel::DeselectOnMiss(bool Additive) {
  if (Additive) {
    return;
  }
  Selection.Clear();
}

void InteractionWheel::ReceiveRayResult(NotifyRayResult Event) {
  Position = Event.Pos;
  SurfaceNormal = Event.SurfaceNormal;
  SelectedBiome = Event.Biome;

  if (!PreviewActive) {
    // not mid hold-to-preview drag, so this is a genuine click (LMB / right
    // trigger) landing on empty ground rather than a drag-hover tick -
    // same "didn't click a friendly unit" case as ShareInfoSelectedEntReceive
    DeselectOnMiss(Event.Additive);
    return;
  }

  Selection.PruneDestroyed();

  // one line is drawn for the group, from whichever unit leads it. every
  // selected unit is going to the same tile, and the render side has a single
  // shared preview object (RenderSystem::PathPreviewLine), so drawing a
  // second would only overwrite the first
  const entt::entity PreviewFor = Selection.Lead();
  if (PreviewFor == entt::null) {
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
      PreviewFor, Position, SelectedBiome, GamePlayer));
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

// the preview only ever showed the lead unit's route, but the order was given
// to the whole group, so every selected unit is sent. each is validated and
// pathed on its own by ECSHelper::ValidateEntityMovement, so anything in the
// set that cant make the trip - a city, a land unit sent to sea - simply
// doesnt move rather than spoiling the order for the rest
void InteractionWheel::CommitPathPreview(CommitPathPreviewEvent Event) {
  Selection.PruneDestroyed();

  for (entt::entity Selected : Selection.Entities()) {
    Factory->FactoryQueue->Enqueue(TryMoveEntityEvent(
        Selected, Position, SurfaceNormal, SelectedBiome, GamePlayer));
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
      [](EventQueue& queue,float x, float y) { queue.Enqueue(CallBackACommand{}); },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_B" + std::to_string(ThreadID),
      [](EventQueue& queue, float x, float y) {
        queue.Enqueue(CallBackBCommand{});
      },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_C" + std::to_string(ThreadID),
      [](EventQueue& queue, float x, float y) {
        queue.Enqueue(CallBackCCommand{});
      },
      ForeignNotifQueue));
  RS.RenderQueue->Enqueue(RegisterOnPressCallBackEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_D" + std::to_string(ThreadID),
      [](EventQueue& queue, float x, float y) {
        queue.Enqueue(CallBackDCommand{});
      },
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
  Selection.PruneDestroyed();

  for (entt::entity Selected : Selection.Entities()) {
    Factory->FactoryQueue->Enqueue(TryDestroyEntityEvent(Selected));
  }

  // the handles are about to stop being valid, and an unselect on a destroyed
  // entity has nothing left to put a material back on, so the set is emptied
  // here rather than left to PruneDestroyed to notice later
  Selection.Reset();
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
