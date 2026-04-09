#include "Cursor.h"

Cursor::Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice,
               int CursorNumber)
    : ForeignBus(ParentBus),
      DeviceState(ControlingDevice),
      CursorID(CursorNumber) {
  RenderSystem& Renderer = RenderSystem::GetInstance();

  Renderer.RenderQueue->Enqueue(CreateOverlayEvent(
      "Cursor_" + std::to_string(CursorID), DeviceState->ManagedDevice));
  std::vector<float> AbsoluteViewPortDimensions =
      DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};

  // use the windows cursor only, this class is however still needed for the
  // windows cursor to select things
  if (DeviceState->ManagedDevice->InputType != InputDeviceType::KBM) {
    Renderer.RenderQueue->Enqueue(
        OverlayAddBoxEvent(RelativeCursorPosition, {0.01f, 0.01f},
                           "Cursor_element_" + std::to_string(CursorID), "RED",
                           "Cursor_" + std::to_string(CursorID)));
  }
  ParentBus->Subscribe<ChangeCursorVisibilityEvent>(
      std::bind(&Cursor::ChangeVisibility, this, std::placeholders::_1));
}

void Cursor::ChangeVisibility(ChangeCursorVisibilityEvent Event) {
  if (DeviceState->ManagedDevice->InputType == InputDeviceType::KBM) {
    return;
  }
  std::vector<float> AbsoluteViewPortDimensions =
      DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};
  RenderSystem& Renderer = RenderSystem::GetInstance();
  if (!Event.Visibile) {
    Renderer.RenderQueue->Enqueue(
        OverlayEditPanelEvent("Cursor_element_" + std::to_string(CursorID),
                              "Cursor_" + std::to_string(CursorID), {0.f, 0.f},
                              RelativeCursorPosition, "USE_OLD"));
  } else {
    Renderer.RenderQueue->Enqueue(OverlayEditPanelEvent(
        "Cursor_element_" + std::to_string(CursorID),
        "Cursor_" + std::to_string(CursorID), {0.01f, 0.01f},
        RelativeCursorPosition, "USE_OLD"));
  }
}

void Cursor::Update() {
  RenderSystem& Renderer = RenderSystem::GetInstance();
  std::vector<float> AbsoluteViewPortDimensions =
      DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};

  if (DeviceState->ManagedDevice->InputType != InputDeviceType::KBM) {
    Renderer.RenderQueue->Enqueue(OverlayEditPanelEvent(
        "Cursor_element_" + std::to_string(CursorID),
        "Cursor_" + std::to_string(CursorID), {0.01f, 0.01f},
        RelativeCursorPosition, "USE_OLD"));
  }
}
