//Copyright © 2025 Henry Frodsham
#include "ErrorReporter.h"


ErrorReporter::ErrorReporter() {
	ErrorBus = EventBus();
	ErrorQueue = EventQueue();


	// every error logged automatically
	// errors that can be handled have other handlers bound
	ErrorBus.Subscribe<Error>(std::bind(&ErrorReporter::ErrorOutput, this, std::placeholders::_1));
}

// main handler for errors, unless resolution is possible and another function is used
void ErrorReporter::ErrorOutput(Error ErrorToReport) {
	std::cout << "[ERROR " << uint32_t(ErrorToReport.Code)
		<< " ] AT" << ErrorToReport.Origin << " \n SEVERITY: "
		<< uint16_t(ErrorToReport.Level) << "\n"
		<< "OUTPUT: " << ErrorToReport.Message;
}

