//Copyright © 2025 Henry Frodsham
#include "ErrorReporter.h"

#define BRIGHT_RED     "\033[91m"
#define RESET   "\033[0m"

ErrorReporter::ErrorReporter(): ErrorBus(),
	ErrorQueue(&ErrorBus){

	// every error logged automatically
	// errors that can be handled have other handlers bound
	ErrorBus.Subscribe<Error>(std::bind(&ErrorReporter::ErrorOutput, this, std::placeholders::_1));
}

// main handler for errors, unless resolution is possible and another function is used
void ErrorReporter::ErrorOutput(Error ErrorToReport) {
	std::cout << BRIGHT_RED << "[ERROR " << uint32_t(ErrorToReport.Code)
		<< "]" << RESET << " At " << ErrorToReport.Origin << ", Severity: "
		<< uint16_t(ErrorToReport.Level)
		<< " Details: " << ErrorToReport.Message;
}

