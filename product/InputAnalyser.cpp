//Copyright © 2025 Henry Frodsham
#include "InputAnalyser.h"

InputAnalyser::InputAnalyser()
{
	MetricBus = EventBus();
	MetricQueue = EventQueue(&MetricBus);
	MetricError = ErrorReporter();
}

void InputAnalyser::RegisterNew(InputTranslator* NewTranslator) {
	Translators.push_back(NewTranslator);
	InputMetrics.emplace(InputMetric(0,0,0), NewTranslator);

	RenderSystem& Renderer = RenderSystem::GetInstance();
	
	Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
		{0.f,0.f + float((Translators.size()-1) / 20.f) }
		, {1.f,1.f}
		, "M_DEV_" + std::to_string(Translators.size())
		, "RED"
		, "DEBUG"
		, "DEVICE " + std::to_string(Translators.size())
	));
	Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
		{ 0.02f,0.02f + float((Translators.size()-1) / 20.f) }
		, { 1.f,1.f }
		, "M_MET_" + std::to_string(Translators.size())
		, "RED"
		, "DEBUG"
		, "PRESSED BUTTONS/KEYS " + std::to_string(Translators.size())
	));
	Renderer.RenderQueue->Enqueue(OverlayAddTextEvent(
		{ 0.02f,0.04f + float((Translators.size()-1) / 20.f) }
		, { 1.f,1.f }
		, "M_MET_C" + std::to_string(Translators.size())
		, "RED"
		, "DEBUG"
		, "AXIS/CURSOR " + std::to_string(Translators.size())
	));
}

void InputAnalyser::Update() {
	MetricQueue.Dispatch();
	MetricError.Dispatch();
	RenderSystem& Renderer = RenderSystem::GetInstance();

	for (int i = 0; i < Translators.size(); i++) {
		InputTranslator* TranslatorToUpdate = Translators[i];
		int NumKeys = TranslatorToUpdate->GetNumPressedKeys();
		std::vector<float> VecPos = TranslatorToUpdate->GetCurrentAxis();

		Renderer.RenderQueue->Enqueue(OverlayEditTextEvent(
			 "M_MET_" + std::to_string(i+1)
			, "DEBUG"
			,{ -1.f,-1.f }
			, { -1.f,-1.f }
			, "USE_OLD"
			, "PRESSED BUTTONS/KEYS " + std::to_string(NumKeys)
		));
		Renderer.RenderQueue->Enqueue(OverlayEditTextEvent(
			"M_MET_C" + std::to_string(i + 1)
			, "DEBUG"
			, { -1.f,-1.f }
			, { -1.f,-1.f }
			, "USE_OLD"
			, fmt::format("AXIS X : {} AXIS Y : {}", VecPos[0], VecPos[1])
		));
	}
}
InputAnalyser& InputAnalyser::GetInstance() {
	static InputAnalyser Instance;
	return Instance;
}