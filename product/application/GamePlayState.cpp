// Copyright (c) 2025 Henry Frodsham
#include "GamePlayState.h"

GameState::GameState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}
void GameState::Init() {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));

  // debug slider, printed to stdout on change so it can be pointed at
  // whatever's being tuned that session. ThreadId 1 matches the primary
  // (KBM) player's InputTranslator (see InstanceOverseer::CreateInstance),
  // the same convention MenuState's PlayButton uses, so presses from the
  // main device are actually routed to it
  DebugSlider = new GenericSlider(
      "debug_slider", "debug", {0.5f, 0.9f}, {0.3f, 0.03f}, 100.f, 0.2f,
      nullptr,
      [](EventQueue& Queue, float MouseX, float MouseY) {
        std::cout << "[debug slider] MouseX=" << MouseX << std::endl;
      },
      AppQueue, 1);

  // starts hidden - Init() runs before the app ever enters GAME state, and
  // AddBox makes new overlay elements visible by default
  DebugSlider->ChangeVisibility(false);
}
// called same tick as a change state event
void GameState::OnChangeState(ChangeStateEvent Event) {
  RenderSystem& Rs = RenderSystem::GetInstance();
  if (Event.NewState == AppState::GAME) {
    Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(true));
    DebugSlider->ChangeVisibility(true);
  } else {
    // hide the world, freeze everything, maybe save?
    Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
    DebugSlider->ChangeVisibility(false);
  }
}
