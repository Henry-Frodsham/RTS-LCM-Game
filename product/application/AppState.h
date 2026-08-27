// Copyright (c) 2025 Henry Frodsham
#pragma once
// the different states the game can be in, used to prevent too much logic being
// processed when unnecessary
// LOADING is the gap between pressing play and the game being playable - the
// globe is built there rather than at startup, so it is a state of its own
// with the progress bar as its UI
enum AppState { GAME, MENU, PAUSE, MAIN_OPTION, GLOBE_OPTION, LOADING };

// split screen means a state is not the application's, it is one player's - one
// player sitting in their options page has no bearing on where anybody else is.
// so every state, and every request to change state, names the instance it
// belongs to
//
// this owner is the instance's thread id, the same number its viewport, overlay
// and cursor are keyed by, so a state and the UI it draws always agree on who
// they are for

// addressed to every instance at once, for a change that really is the whole
// application's - starting the game. a state owned by it is application wide
// and so only ever moves on one of those broadcasts
constexpr int StateOwnerAll = 0;

// the first instance registered, which is the keyboard and mouse. it owns the
// main menu and main options pages, and is the only instance that can start the
// game, so it is the only one whose menu offers play rather than ready
constexpr int PrimaryStateOwner = 1;
