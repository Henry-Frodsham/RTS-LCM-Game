// Copyright (c) 2026 Henry Frodsham
#include "MainOptionState.h"

#include <string>

MainOptionState::MainOptionState(EventQueue* CallBacksQueue)
    : BaseOptionState(CallBacksQueue, AppState::MAIN_OPTION, nullptr,
                      PrimaryStateOwner, "main_options") {}

void MainOptionState::OnPageEnter() {
  RenderSystem& Rs = RenderSystem::GetInstance();
  Rs.RenderQueue->Enqueue(ChangeGlobeVisibilityEvent(false));
}

// every numeric key in the three config files, minus the ones this page has no
// element for yet - the flags are waiting on a tickbox, and the key binds and
// material names are text rather than a value on a track
//
// the ranges are the sensible span for each setting rather than zero to the
// default, since a window 0 pixels wide or a globe of 0 subdivisions is not a
// setting anybody wants to be one drag away from
void MainOptionState::OnBindSettings() {
  // ---- video, the whole application's ----
  // the window is built at startup and everything on screen is laid out
  // against the size it was built at, so a new one is what the next launch
  // opens with rather than something that happens under the page it is set on
  BindSlider<unsigned int>("window_width", "Window Width", "VideoSettings",
                           "WindowWidth", 640.f, 3840.f);
  BindSlider<unsigned int>("window_height", "Window Height", "VideoSettings",
                           "WindowHeight", 480.f, 2160.f);


  // ---- input, this player's ----
  // the keyboard and mouse is instance 1, and its InputTranslator reads the
  // file named after it, so this page reaches for that one by name rather than
  // the shared file the settings above live in
  ConfigManager* Input =
      Config("InputSettings", std::to_string(PrimaryStateOwner));

  BindSlider<float>("cursor_sensitivity", "Cursor Sensitivity", Input,
                    "CursorSensitivity", 100.f, 3000.f);
  BindSlider<float>("orbit_sensitivity", "Orbit Sensitivity", Input,
                    "ControllerOrbitSensitivity", 0.5f, 20.f);
  BindSlider<float>("joystick_deadzone", "Stick Deadzone", Input,
                    "JoystickDeadzone", 0.f, 0.9f);
  BindSlider<float>("trigger_threshold", "Trigger Threshold", Input,
                    "TriggerThreshold", 0.f, 0.9f);
  BindSlider<float>("motion_scale", "Motion Scale", Input,
                    "RelativeMotionScale", 10.f, 400.f);
  BindSlider<float>("drag_threshold", "Drag Threshold", Input,
                    "RightDragThresholdPixels", 0.f, 40.f);
  BindSlider<float>("shoulder_zoom", "Shoulder Zoom", Input,
                    "ShoulderZoomNotchesPerSecond", 1.f, 100.f);

  // ---- the reconnect prompt's layout, this player's ----
  BindComponentSlider("prompt_x", "Prompt X", Input, "PromptPosition", 0, 0.f,
                      1.f);
  BindComponentSlider("prompt_y", "Prompt Y", Input, "PromptPosition", 1, 0.f,
                      1.f);
  BindComponentSlider("prompt_w", "Prompt Width", Input, "PromptDimensions", 0,
                      0.05f, 1.f);
  BindComponentSlider("prompt_h", "Prompt Height", Input, "PromptDimensions",
                      1, 0.01f, 0.3f);

  // still unbound, and deliberately: FullScreen and
  // RightClickOpensContextWheel are waiting on a tickbox element, and
  // WindowName, KeyBindUse / Back / Context and PromptBorderMaterial /
  // PromptTextMaterial are text rather than a number. BindToggle already
  // covers the flags if a button captioned on / off will do in the meantime
}
