// Copyright (c) 2026 Henry Frodsham
#include "GlobeOptionState.h"

GlobeOptionState::GlobeOptionState(EventQueue* CallBacksQueue)
    : BaseOptionState(CallBacksQueue, AppState::GLOBE_OPTION, nullptr,
                      PrimaryStateOwner, "globe_options") {}

void GlobeOptionState::OnPageEnter() {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
}

// the ranges are the sensible span for each setting rather than zero to the
// default, since a globe of 0 subdivisions is not a setting anybody wants to
// be one drag away from
void GlobeOptionState::OnBindSettings() {
  BindSlider<unsigned int>("subdivisions", "Subdivisions", "GlobeSettings",
                           "NumSubdivisions", 10.f, 300.f);
  BindSlider<unsigned int>("seed", "World Seed", "GlobeSettings",
                           "CreationSeed", 0.f, 10000.f);
  BindSlider<float>("radius", "Planet Radius", "GlobeSettings", "PlanetRadius",
                    0.1f, 2.f);
}
