// Copyright (c) 2025 Henry Frodsham
#include "MenuState.h"

#include <vector>

MenuState::MenuState(EventQueue* CallBacksQueue)
    : BaseState(CallBacksQueue, AppState::MENU, PrimaryStateOwner),
      PlayButton(nullptr), OptionButton(nullptr), WorldGenerationButton(nullptr) {}

void MenuState::OnInit() {
  // starting the game is the one thing here that is not just this instance's,
  // so it is the one press that is addressed to everybody. it leads to the
  // loading state rather than straight into the game, because the world it
  // needs is built from this press rather than from startup
  PlayButton = CreateElement<GenericButton>(
      "menu_play", "play", std::vector<float>{0.5f, 0.5f}, nullptr,
      [](EventQueue& Queue, float MouseX, float MouseY) {
        Queue.Enqueue(ChangeStateEvent{AppState::LOADING});
      },
      AppQueue, PrimaryStateOwner);
  OptionButton = CreateElement<GenericButton>(
      "menu_option", "options", std::vector<float>{0.5f, 0.56f}, nullptr,
      [](EventQueue& Queue, float MouseX, float MouseY) {
        Queue.Enqueue(ChangeStateEvent{AppState::MAIN_OPTION,
                                       PrimaryStateOwner});
      },
      AppQueue, PrimaryStateOwner);

  WorldGenerationButton = CreateElement<GenericButton>(
      "world_generation", "world generation", std::vector<float>{0.5f, 0.62f},
      nullptr,
      [](EventQueue& Queue, float MouseX, float MouseY) {
        Queue.Enqueue(
            ChangeStateEvent{AppState::GLOBE_OPTION, PrimaryStateOwner});
      },
      AppQueue, PrimaryStateOwner);
}
