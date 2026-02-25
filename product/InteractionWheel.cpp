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
      OverlayAddBoxEvent({0.f, 0.f}, {0.01f, 0.01f},
                         "interaction_wheel_" + std::to_string(ThreadID), "RED",
                         "UI_Overlay_" + std::to_string(ThreadID)));

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_" + std::to_string(ThreadID), false));
  Visibility = false;
}

void InteractionWheel::UpdateAndWarmupContext() {

}

void InteractionWheel::OnContextActionCommand(ContextActionCommand Cmd) {
  ActionContext Context = Cmd.Context;

  std::vector<float> Dimensions = DeviceState->GetViewPortDimensions();
  Visibility = !Visibility;
  RenderSystem& RS = RenderSystem::GetInstance();
  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(ThreadID),
      "interaction_wheel_" + std::to_string(ThreadID), Visibility));

  RS.RenderQueue->Enqueue(OverlayEditPanelEvent(
      "interaction_wheel_" + std::to_string(ThreadID),
      "UI_Overlay_" + std::to_string(ThreadID), {-1, -1},
      {Context.MouseX / Dimensions[0], Context.MouseY / Dimensions[1]}));
}