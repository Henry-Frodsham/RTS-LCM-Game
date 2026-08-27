// Copyright (c) 2026 Henry Frodsham
#pragma once
#include "BaseState.h"
#include "GenericLoadingProgress.h"
#include "GlobeEvent.h"
#include "RenderSystem.h"

// the gap between pressing play and the game being playable. the globe is
// built here rather than at startup, so that the settings behind it are still
// editable right up until the moment the world is made
//
// the building itself is done on a worker thread inside GlobeInterface, so
// this state does not wait on anything - it asks for the world once on the way
// in, draws how far along it is every frame like any other active state, and
// moves the whole application into the game once it is there
class LoadingState : public BaseState {
 public:
  explicit LoadingState(EventQueue* CallBacksQueue);

 protected:
  void OnInit() override;
  void OnEnter() override;
  void OnMaintain() override;

 private:
  GenericLoadingProgress* ProgressBar;

  // whether the build has been seen to start. a build that has started and is
  // no longer running without having produced a world is one that failed, and
  // there is no game to go to - but it takes a frame or two for the request to
  // reach the render thread, so the two cannot be told apart without this
  bool GenerationStarted;
};
