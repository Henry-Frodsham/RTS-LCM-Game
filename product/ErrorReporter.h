//Copyright © 2025 Henry Frodsham
#pragma once
#include "ErrorManifest.h"
#include "EventQueue.h"
#include "EventBus.h"
#include <iostream>
#include <functional>

class ErrorReporter {
public:
	ErrorReporter();
	~ErrorReporter(){}

	EventQueue ErrorQueue;

private:

	void ErrorOutput(Error ErrorToReport);
	EventBus ErrorBus;
};