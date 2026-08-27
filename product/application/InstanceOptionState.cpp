// Copyright (c) 2026 Henry Frodsham
#include "InstanceOptionState.h"

#include <string>

InstanceOptionState::InstanceOptionState(EventQueue* CallBacksQueue,
                                         InputDevice* InstanceDevice,
                                         int InstanceId)
    : BaseOptionState(CallBacksQueue, AppState::MAIN_OPTION, InstanceDevice,
                      InstanceId, "instance_options",
                      std::to_string(InstanceId)) {}

// only the settings that are one player's belong here - the window and the
// world are the whole application's and stay on MainOptionState. every config
// named below is already this instance's own, so a key needs no more than its
// file's base name and the key itself, and applying writes
// config/custom/InputSettings<N>.json and reloads that instance's translator
void InstanceOptionState::OnBindSettings() {
  BindSlider<float>("cursor_sensitivity", "Cursor Sensitivity",
                    "InputSettings", "CursorSensitivity", 100.f, 3000.f);
  BindSlider<float>("orbit_sensitivity", "Orbit Sensitivity", "InputSettings",
                    "ControllerOrbitSensitivity", 0.5f, 20.f);
  BindSlider<float>("joystick_deadzone", "Stick Deadzone", "InputSettings",
                    "JoystickDeadzone", 0.f, 0.9f);
  BindSlider<float>("trigger_threshold", "Trigger Threshold", "InputSettings",
                    "TriggerThreshold", 0.f, 0.9f);
  BindSlider<float>("motion_scale", "Motion Scale", "InputSettings",
                    "RelativeMotionScale", 10.f, 400.f);
  BindSlider<float>("drag_threshold", "Drag Threshold", "InputSettings",
                    "RightDragThresholdPixels", 0.f, 40.f);
  BindSlider<float>("shoulder_zoom", "Shoulder Zoom", "InputSettings",
                    "ShoulderZoomNotchesPerSecond", 1.f, 100.f);

  BindComponentSlider("prompt_x", "Prompt X", "InputSettings",
                      "PromptPosition", 0, 0.f, 1.f);
  BindComponentSlider("prompt_y", "Prompt Y", "InputSettings",
                      "PromptPosition", 1, 0.f, 1.f);
  BindComponentSlider("prompt_w", "Prompt Width", "InputSettings",
                      "PromptDimensions", 0, 0.05f, 1.f);
  BindComponentSlider("prompt_h", "Prompt Height", "InputSettings",
                      "PromptDimensions", 1, 0.01f, 0.3f);

  // RightClickOpensContextWheel is waiting on a tickbox, and the key binds and
  // the prompt's materials are text rather than a number
}
