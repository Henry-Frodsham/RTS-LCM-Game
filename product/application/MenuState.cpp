// Copyright © 2025 Henry Frodsham
#include "MenuState.h"
MenuState::MenuState(EventQueue* CallBacksQueue) : AppQueue(CallBacksQueue) {

}
void MenuState::Init() { }

void MenuState::OnChangeState(ChangeStateEvent Event) {
  if (Event.NewState == AppState::MENU) {
  } else {
	  //hide the play button, or any other buttons
  }
}