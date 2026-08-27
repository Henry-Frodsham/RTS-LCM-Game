// Copyright (c) 2026 Henry Frodsham
#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "AppState.h"
#include "EventQueue.h"
#include "StateEvent.h"
#include "UIElement.h"

// shared behaviour for every application state. owns the state's UI and drives
// its whole lifecycle - elements are hidden until the app enters the state,
// shown while it is active, and rescaled every frame - so a derived state only
// has to describe what it is made of
//
// a state also belongs to one instance, and ignores any change of state
// addressed to a different one, so several states are active at once in split
// screen - one per instance, plus whatever is application wide
class BaseState {
 public:
  BaseState(EventQueue* CallBacksQueue, AppState StateOwned,
            int Owner = StateOwnerAll);
  virtual ~BaseState() = default;

  BaseState(const BaseState&) = delete;
  BaseState& operator=(const BaseState&) = delete;

  void Init();
  void OnChangeState(ChangeStateEvent Event);
  void MaintainScaling();

  AppState GetOwnedState() const;
  int GetOwner() const;
  bool IsActive() const;

 protected:
  // build the state's UI here, with CreateElement
  virtual void OnInit() {}

  // the app has just entered / left this state. visibility is already handled
  virtual void OnEnter() {}
  virtual void OnExit() {}

  // once a frame, for as long as this state is the active one. for a state
  // that is watching something rather than waiting to be pressed - work going
  // on elsewhere whose progress it is drawing
  virtual void OnMaintain() {}

  // the only way to make an element, so an element can never be created and
  // then forgotten about by the automatic show/hide and scaling
  template <typename ElementType, typename... ArgTypes>
  ElementType* CreateElement(ArgTypes&&... Args) {
    std::unique_ptr<ElementType> Owned =
        std::make_unique<ElementType>(std::forward<ArgTypes>(Args)...);

    ElementType* Created = Owned.get();
    Elements.push_back(std::move(Owned));

    Created->ChangeVisibility(Active);
    return Created;
  }

  EventQueue* AppQueue;

 private:
  void SetElementVisibility(bool Visible);

  std::vector<std::unique_ptr<UIElement>> Elements;

  AppState OwnedState;
  int OwnedBy;
  bool Active;
};
