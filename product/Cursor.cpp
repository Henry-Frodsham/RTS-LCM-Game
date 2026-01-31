#include "Cursor.h"

Cursor::Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice, int CursorNumber)
    : ForeignBus(ParentBus), DeviceState(ControlingDevice), CursorID(CursorNumber) {
  RenderSystem& Renderer = RenderSystem::GetInstance();

  Renderer.RenderQueue->Enqueue(CreateOverlayEvent(
      "Cursor_" + std::to_string(CursorID), DeviceState->ManagedDevice));
  std::vector<float> AbsoluteViewPortDimensions = DeviceState->GetViewPortDimensions();
  std::vector<float> AbsoluteCursorPosition = DeviceState->GetCurrentAxis();
  std::vector<float> RelativeCursorPosition = std::vector<float>{
      AbsoluteCursorPosition[0] / AbsoluteViewPortDimensions[0],
      AbsoluteCursorPosition[1] / AbsoluteViewPortDimensions[1]};

  Renderer.RenderQueue->Enqueue(OverlayAddBoxEvent(RelativeCursorPosition, {0.01f, 0.01f},
                         "Cursor_element_" + std::to_string(CursorID),
                         "RED", "Cursor_" + std::to_string(CursorID)));
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
      OverlayEditPanelEvent("Cursor_element_" + std::to_string(CursorID),
                            "Cursor_" + std::to_string(CursorID),
                            {0.01f, 0.01f}, RelativeCursorPosition, "USE_OLD"));
}
