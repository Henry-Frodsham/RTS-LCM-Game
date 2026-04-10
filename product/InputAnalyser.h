// Copyright (c) 2025 Henry Frodsham
#pragma once
#include <unordered_map>
#include <vector>

#include "ErrorReporter.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "InputDevice.h"
#include "InputTranslator.h"
#include "Metrics.h"
#include "RenderSystem.h"

// a debug overlay visible only when built in debug
// shows the activity of input devices

// class used to gather metrics of registered input devices and manage a unique
// overlay for each
class InputAnalyser {
 private:
  InputAnalyser();

  std::vector<InputTranslator*> Translators;
  std::unordered_map<InputMetric, InputTranslator*> InputMetrics;

  EventBus* MetricBus;

  ErrorReporter* MetricError;

 public:
  static InputAnalyser& GetInstance();

  void RegisterNew(InputTranslator* NewTranslator);

  void Update(float DeltaTime);

  EventQueue* MetricQueue;
};
