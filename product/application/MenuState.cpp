// Copyright © 2025 Henry Frodsham
#include "MenuState.h"
MenuState::MenuState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {}
void MenuState::Init() {
  PlayButton = new GenericButton(
      "menu_play", "play", {0.5f, 0.5f}, nullptr,
      [](EventQueue& queue) {
        queue.Enqueue(ChangeStateEvent{AppState::GAME});
      },
      AppQueue, 1);
}

void MenuState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::MENU) {
    PlayButton->ChangeVisibility(true);
  } else {
    // hide the play button, or any other buttons
    PlayButton->ChangeVisibility(false);
  }
}
