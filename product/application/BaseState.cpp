// Copyright (c) 2026 Henry Frodsham
#include "BaseState.h"

#include <memory>

BaseState::BaseState(EventQueue* CallBacksQueue, AppState StateOwned,
                     int Owner)
    : AppQueue(CallBacksQueue),
      OwnedState(StateOwned),
      OwnedBy(Owner),
      Active(false) {}

void BaseState::Init() {
  OnInit();

  // nothing is on screen until the app actually enters this state, rather than
  // relying on whatever visibility an element happened to be created with
  SetElementVisibility(false);
}

void BaseState::OnChangeState(ChangeStateEvent Event) {
  // where another instance has gone says nothing about where this one is, so
  // only a change addressed to this state's instance, or to everyone, counts
  if (Event.Owner != StateOwnerAll && Event.Owner != OwnedBy) {
    return;
  }

  const bool NowActive = (Event.NewState == OwnedState);

  if (NowActive == Active) {
    return;
  }

  Active = NowActive;
  SetElementVisibility(Active);

  if (Active) {
    OnEnter();
  } else {
    OnExit();
  }
}

void BaseState::MaintainScaling() {
  for (const std::unique_ptr<UIElement>& Element : Elements) {
    Element->MaintainScaling();
  }

  OnMaintain();
}

AppState BaseState::GetOwnedState() const { return OwnedState; }

int BaseState::GetOwner() const { return OwnedBy; }

bool BaseState::IsActive() const { return Active; }

void BaseState::SetElementVisibility(bool Visible) {
  for (const std::unique_ptr<UIElement>& Element : Elements) {
    Element->ChangeVisibility(Visible);
  }
}
