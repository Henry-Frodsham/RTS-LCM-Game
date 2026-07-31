// Copyright (c) 2025 Henry Frodsham
#include "PlayerUI.h"

PlayerUI::PlayerUI(Player* player, int ThreadId)
    : GamePlayer(player), Id(ThreadId) {
  PlayerUIBus = new EventBus();
  PlayerUIQueue = new EventQueue();

  SetUp();
}

void PlayerUI::SetUp() {
  RenderSystem& RS = RenderSystem::GetInstance();
  /*
  RS.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.14f}, {1.f, 1.f}, "UnitsA_ui_" + std::to_string(Id), "RED",
      "UI_Overlay_" + std::to_string(Id), "Units Available - 0"));
  RS.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.16f}, {1.f, 1.f}, "CitiesA_ui_" + std::to_string(Id), "RED",
      "UI_Overlay_" + std::to_string(Id), "Cities Available - 0"));
  RS.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.18f}, {1.f, 1.f}, "prod_ui_" + std::to_string(Id), "RED",
      "UI_Overlay_" + std::to_string(Id), "Unit Production / m - 0"));
  RS.RenderQueue->Enqueue(OverlayAddTextEvent(
      {0.02f, 0.2f}, {1.f, 1.f}, "prog_ui_" + std::to_string(Id), "RED",
      "UI_Overlay_" + std::to_string(Id), "Unit Production Progress % - 0"));
  */
}

void PlayerUI::Update() {
  PlayerUIQueue->Dispatch();
  /*
  RenderSystem& RS = RenderSystem::GetInstance();
  if (GamePlayer->AvailableUnits != KnownAUnits) {
    RS.RenderQueue->Enqueue(OverlayEditTextEvent(
        "UnitsA_ui_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        "Units Available -  " + std::to_string(GamePlayer->AvailableUnits)));
    KnownAUnits = GamePlayer->AvailableUnits;
  }
  if (GamePlayer->AvailableCities != KnownACities) {
    RS.RenderQueue->Enqueue(OverlayEditTextEvent(
        "CitiesA_ui_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        "Cities Available -  " + std::to_string(GamePlayer->AvailableCities)));
    KnownACities = GamePlayer->AvailableCities;
  }
  if (GamePlayer->UnitProduction != KnownProd) {
    RS.RenderQueue->Enqueue(OverlayEditTextEvent(
        "prod_ui_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        "Unit Production / m - " + std::to_string(GamePlayer->UnitProduction)));
    KnownProd = GamePlayer->UnitProduction;
  }
  if (GamePlayer->UnitProdProg != KnownProg) {
    RS.RenderQueue->Enqueue(OverlayEditTextEvent(
        "prog_ui_" + std::to_string(Id), "UI_Overlay_" + std::to_string(Id),
        {-1.f, -1.f}, {-1.f, -1.f}, "USE_OLD",
        "Unit Production Progress % - " +
            std::to_string(GamePlayer->UnitProdProg)));
    KnownProg = GamePlayer->UnitProdProg;
  }
  */
}
