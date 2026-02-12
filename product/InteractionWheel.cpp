#include "InteractionWheel.h"

InteractionWheel::InteractionWheel(InputTranslator* Device, EventBus* Bus,
                                   int ThreadNum) {
  DeviceState = Device;
  IssuingBus = Bus;
  ThreadID = ThreadNum;
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(OverlayAddBoxEvent(
      {0.f, 0.f}, {0.01f, 0.01f}, "UI_Overlay_" + std::to_string(ThreadID),
      "RED", "interaction_wheel_" + std::to_string(ThreadID)));
}

void InteractionWheel::ToggleVisibility(ToggleInteractionWheelEvent Event) {}
