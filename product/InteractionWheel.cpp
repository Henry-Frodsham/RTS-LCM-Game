#include "InteractionWheel.h"

InteractionWheel::InteractionWheel(InputTranslator* Device, int ThreadNum,
                                   ECSHelper* Interactor, Player* Play) {
  DeviceState = Device;
  ThreadID = ThreadNum;
  Factory = Interactor;
  RenderSystem& RS = RenderSystem::GetInstance();
  ForeignNotifBus = new EventBus();
  ForeignNotifQueue = new EventQueue(ForeignNotifBus);
  SelectedEntity = nullptr;
  GamePlayer = Play;
  LatLonR = Ogre::Vector2f();
  Position = Ogre::Vector3f();
  Device->ActionBus->Subscribe<ContextActionCommand>(std::bind(
      &InteractionWheel::OnContextActionCommand, this, std::placeholders::_1));
  ForeignNotifBus->Subscribe<NotifySelectedEntity>(
      std::bind(&InteractionWheel::ShareInfoSelectedEntReceive, this,
                std::placeholders::_1));
  ForeignNotifBus->Subscribe<NotifyPosEvent>(std::bind(
      &InteractionWheel::ShareInfoHitPosReceive, this,
                std::placeholders::_1));
  ForeignNotifBus->Subscribe<NotifyLatLonEvent>(std::bind(
      &InteractionWheel::ShareInfoLatLonReceive, this, std::placeholders::_1));
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
      "PLACE",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_B" + std::to_string(ThreadID),
      "interaction_wheel_B_text_" + std::to_string(ThreadID),
      "DEST",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_C" + std::to_string(ThreadID),
      "interaction_wheel_C_text_" + std::to_string(ThreadID),
      "MOVE",
      "WHITE",
      {0.f, 0.f},
      {1.f, 1.f}});
  RS.RenderQueue->Enqueue(OverlayAddTextToPanelEvent{
      "interaction_wheel_D" + std::to_string(ThreadID),
      "interaction_wheel_D_text_" + std::to_string(ThreadID),
      "IDK",
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

void InteractionWheel::ShareInfoSelectedEntReceive(NotifySelectedEntity Event) {
  SelectedEntity = Event.Entity;
}
void InteractionWheel::ShareInfoLatLonReceive(NotifyLatLonEvent Event) {
  LatLonR = Event.LatLon;
}
void InteractionWheel::ShareInfoHitPosReceive(NotifyPosEvent Event) {
  Position = Event.Pos;
}
void InteractionWheel::OnContextActionCommand(ContextActionCommand Cmd) {
  ActionContext Context = Cmd.Context;

  std::vector<float> Dimensions = DeviceState->GetViewPortDimensions();
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
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_A" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
      {Context.MouseX / Dimensions[0], Context.MouseY / Dimensions[1]}));
  RS.RenderQueue->Enqueue(
      OverlayEditPanelEvent("interaction_wheel_B" + std::to_string(ThreadID),
                            "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
                            {(Context.MouseX / Dimensions[0]) + 0.03f,
                             Context.MouseY / Dimensions[1]}));
  RS.RenderQueue->Enqueue(
      OverlayEditPanelEvent("interaction_wheel_C" + std::to_string(ThreadID),
                            "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
                            {Context.MouseX / Dimensions[0],
                             (Context.MouseY / Dimensions[1]) - 0.03f}));
  RS.RenderQueue->Enqueue(
      OverlayEditPanelEvent("interaction_wheel_D" + std::to_string(ThreadID),
                            "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
                            {(Context.MouseX / Dimensions[0]) + 0.03f,
                             (Context.MouseY / Dimensions[1]) - 0.03f}));

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
    GamePlayer->PlaceCity(Factory,Position);
  }
}
void InteractionWheel::CallBackButtonB(CallBackBCommand Cmd) {
  //destroy

}
void InteractionWheel::CallBackButtonC(CallBackCCommand Cmd) { 
  //move
  if (SelectedEntity != nullptr &&
      SelectedEntity->getParentSceneNode() != nullptr) {
    Factory->FactoryQueue->Enqueue(
        MoveEntityAlongSphericalEvent(SelectedEntity, 1.f, LatLonR));
  }
}
void InteractionWheel::CallBackButtonD(CallBackDCommand Cmd) { 
  //idk
}
