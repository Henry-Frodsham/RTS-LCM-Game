//Copyright © 2025 Henry Frodsham
#pragma once
#include <vector>
#include <unordered_map>
#include "InputDevice.h"
#include "Metrics.h"
#include "InputTranslator.h"
#include "EventBus.h"
#include "EventQueue.h"
#include "RenderSystem.h"
#include "ErrorReporter.h"

//gather metrics of input devices
//then registers and updates an overlay
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

	void Update();

	EventQueue* MetricQueue;



};