#include "Cursor.h"

Cursor::Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice)
    : ForeignBus(ParentBus), DeviceState(ControlingDevice) {
  RenderSystem& Renderer = RenderSystem::GetInstance();

  Renderer.RenderQueue->Enqueue(
      CreateOverlayEvent("Cursor_", DeviceState->ManagedDevice));
  std::vector<float> AbsoluteViewPortDimensions = DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};

  Renderer.RenderQueue->Enqueue(OverlayAddBoxEvent(
      RelativeCursorPosition, {0.01f, 0.01f},
                         "Cursor", "RED", "Cursor_"));
}

void Cursor::ChangeVisibility(ChangeCursorVisibilityEvent Event) {}

void Cursor::Update() {
  RenderSystem& Renderer = RenderSystem::GetInstance();
  std::vector<float> AbsoluteViewPortDimensions =
      DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};

  Renderer.RenderQueue->Enqueue(
      OverlayEditPanelEvent("Cursor", "Cursor_", {0.01f, 0.01f}, RelativeCursorPosition, "USE_OLD"));
}
