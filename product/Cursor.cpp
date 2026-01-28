#include "Cursor.h"

Cursor::Cursor(EventBus* ParentBus, InputTranslator* ControlingDevice) : ForeignBus(ParentBus), DeviceState(ControlingDevice) {

	RenderSystem& Renderer = RenderSystem::GetInstance();

	Renderer.RenderQueue->Enqueue(CreateOverlayEvent("Cursor_", DeviceState->ManagedDevice));

	Renderer.RenderQueue->Enqueue(OverlayAddBoxEvent(DeviceState->GetCurrentAxis(), { 0.01f,0.01f }, "Cursor", "RED", "Cursor_"));
}

void Cursor::ChangeVisibility(ChangeCursorVisibilityEvent Event) {

}


void Cursor::Update() {
	RenderSystem& Renderer = RenderSystem::GetInstance();
	Renderer.RenderQueue->Enqueue(OverlayEditPanelEvent("Cursor","Cursor_", {0.01f,0.01f}, DeviceState->GetCurrentAxis(), "USE_OLD"));

}

