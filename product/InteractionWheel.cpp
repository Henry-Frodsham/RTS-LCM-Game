#include "InteractionWheel.h"

InteractionWheel::InteractionWheel(InputTranslator* Device, int ThreadNum) {
  DeviceState = Device;
  ThreadID = ThreadNum;
  RenderSystem& RS = RenderSystem::GetInstance();

  Device->ActionBus->Subscribe<ContextActionCommand>(std::bind(
      &InteractionWheel::OnContextActionCommand, this, std::placeholders::_1));

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

void InteractionWheel::UpdateAndWarmupContext() {}

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
  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_D" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
      {(Context.MouseX / Dimensions[0]) + 0.03f, (Context.MouseY / Dimensions[1]) - 0.03f}));
}

void InteractionWheel::OnPressActionCommand(PressActionCommand Cmd) {
  ActionContext Context = Cmd.Context;
}
