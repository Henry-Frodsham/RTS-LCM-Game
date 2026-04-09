// Copyright © 2025 Henry Frodsham
#include "InputAnalyser.h"

InputAnalyser::InputAnalyser() {
  MetricBus = new EventBus();
  MetricQueue = new EventQueue(MetricBus);
  MetricError = new ErrorReporter();
}

void InputAnalyser::RegisterNew(InputTranslator* NewTranslator) {
  Translators.push_back(NewTranslator);
  InputMetrics.emplace(InputMetric(0, 0, 0), NewTranslator);

  RenderSystem& Renderer = RenderSystem::GetInstance();

  Renderer.RenderQueue->Enqueue(
      CreateOverlayEvent("DEBUG_" + std::to_string(Translators.size()),
                         NewTranslator->ManagedDevice));

  Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.f, 0.f}, {1.f, 1.f}, "M_DEV_" + std::to_string(Translators.size()),
      "RED", "DEBUG_" + std::to_string(Translators.size()),
      "DEVICE " + std::to_string(Translators.size())));
  Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.02f}, {1.f, 1.f}, "M_MET_" + std::to_string(Translators.size()),
      "RED", "DEBUG_" + std::to_string(Translators.size()),
      "PRESSED BUTTONS/KEYS " + std::to_string(Translators.size())));
  Renderer.RenderQueue->Enqueue(
      OverlayAddTextEvent({0.02f, 0.04f}, {1.f, 1.f},
                          "M_MET_C" + std::to_string(Translators.size()), "RED",
                          "DEBUG_" + std::to_string(Translators.size()),
                          "AXIS/CURSOR " + std::to_string(Translators.size())));
  Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.06f}, {1.f, 1.f}, "M_FPS" + std::to_string(Translators.size()),
      "RED", "DEBUG_" + std::to_string(Translators.size()), "FPS - 0"));
}

void InputAnalyser::Update(float DeltaTime) {
  MetricQueue->Dispatch();
  MetricError->Dispatch();
  RenderSystem& Renderer = RenderSystem::GetInstance();

  for (int i = 0; i < Translators.size(); i++) {
    InputTranslator* TranslatorToUpdate = Translators[i];
    int NumKeys = TranslatorToUpdate->GetNumPressedKeys();
    std::vector<float> VecPos = TranslatorToUpdate->GetCurrentAxis();

    Renderer.RenderQueue->Enqueue(OverlayEditTextEvent(
        "M_MET_" + std::to_string(i + 1), "DEBUG_" + std::to_string(i + 1),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        "PRESSED BUTTONS/KEYS " + std::to_string(NumKeys)));

    Renderer.RenderQueue->Enqueue(OverlayEditTextEvent(
        "M_MET_C" + std::to_string(i + 1), "DEBUG_" + std::to_string(i + 1),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        fmt::format("AXIS X : {} AXIS Y : {}", VecPos[0], VecPos[1])));
    // avoid division by 0
    if (DeltaTime != 0.f) {
      Renderer.RenderQueue->Enqueue(OverlayEditTextEvent(
          "M_FPS" + std::to_string(i + 1), "DEBUG_" + std::to_string(i + 1),
          {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
          fmt::format("FPS - {}", 1 / DeltaTime)));
    }
  }
}
InputAnalyser& InputAnalyser::GetInstance() {
  static InputAnalyser Instance;
  return Instance;
}
