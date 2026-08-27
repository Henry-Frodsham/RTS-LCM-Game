// Copyright (c) 2026 Henry Frodsham
#include "LoadingState.h"

#include <vector>

namespace {
// centred on the screen and wide enough to read a fraction off at a glance
constexpr float BarX = 0.25f;
constexpr float BarY = 0.5f;
constexpr float BarWidth = 0.5f;
constexpr float BarHeight = 0.04f;
}  // namespace

LoadingState::LoadingState(EventQueue* CallBacksQueue)
    : BaseState(CallBacksQueue, AppState::LOADING),
      ProgressBar(nullptr),
      GenerationStarted(false) {}

void LoadingState::OnInit() {
  // the bar is drawn into the keyboard and mouse instance's overlay, the same
  // one the main menu it is replacing uses, rather than into a split screen
  // player's - the world being built is the whole application's
  ProgressBar = CreateElement<GenericLoadingProgress>(
      "loading_globe", std::vector<float>{BarX, BarY},
      std::vector<float>{BarWidth, BarHeight}, nullptr, PrimaryStateOwner);
}

void LoadingState::OnEnter() {
  ProgressBar->SetProgress(0.f);
  GenerationStarted = false;

  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(GenerateGlobeEvent());
}

// the worker reports into an atomic and the render thread publishes the
// finished world, so nothing here blocks - this only reads where both of them
// have got to and reacts to it
void LoadingState::OnMaintain() {
  GlobeInterface* Globe = RenderSystem::GetInstance().GetGlobeInterface();
  const GlobeGenerationState State = Globe->GetGenerationState();

  ProgressBar->SetProgress(Globe->GetGenerationProgress());

  if (State == GlobeGenerationState::Building) {
    GenerationStarted = true;
    return;
  }

  if (State == GlobeGenerationState::Built) {
    AppQueue->Enqueue(ChangeStateEvent{AppState::GAME});
    return;
  }

  // the build gave up and reported why, so there is nowhere to go but back
  if (GenerationStarted) {
    AppQueue->Enqueue(ChangeStateEvent{AppState::MENU});
  }
}
