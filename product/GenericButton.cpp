#include "GenericButton.h"

GenericButton::GenericButton(std::string ButtonName, std::string ButtonText,
                             std::vector<float> Position,
                             InputDevice* DeviceToRespondTo,
                             std::function<void(EventQueue&)> PressCallback,
                             EventQueue* QueueForCallback, int ThreadID)
    : Name(ButtonName),
      Text(ButtonText),
      Pos(Position),
      Device(DeviceToRespondTo),
      CallBackOnPress(PressCallback),
      CallBackQueue(QueueForCallback),
      
      Id(ThreadID) {


  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      CreateOverlayEvent("UI_Overlay_" + std::to_string(Id), Device));

  RS.RenderQueue->Enqueue(
      OverlayAddBoxEvent(Pos, {0.03f, 0.03f}, Name + "_" + std::to_string(Id),
                         "RED", "UI_Overlay_" + std::to_string(ThreadID)));

  RS.RenderQueue->Enqueue(OverlayAddTextEvent(
      Pos, {1.f, 1.f}, Name + "_text_" + std::to_string(Id), "WHITE",
      "UI_Overlay_" + std::to_string(ThreadID), Text));

  RS.RenderQueue->Enqueue(
      RegisterOnPressCallBackEvent("UI_Overlay_" + std::to_string(ThreadID), Name + "_" + std::to_string(Id),
      CallBackOnPress,CallBackQueue));
}

void GenericButton::ChangeVisibility(bool Visible) {
  RenderSystem& RS = RenderSystem::GetInstance();

  RS.RenderQueue->Enqueue(
      ChangeOverlayVisibilityEvent("UI_Overlay_" + std::to_string(Id),
                                   Name + "_" + std::to_string(Id), Visible));

  RS.RenderQueue->Enqueue(ChangeOverlayVisibilityEvent(
      "UI_Overlay_" + std::to_string(Id), Name + "_text_" + std::to_string(Id),
      Visible));
}
