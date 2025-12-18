// Copyright © 2025 Henry Frodsham
#include "ErrorReporter.h"

#define BRIGHT_RED "\033[91m"
#define WHITE "\033[97m"
#define ORANGE "\033[38;5;208m"
#define YELLOW "\033[93m"
#define LIGHT_RED "\033[91m"
#define RESET "\033[0m"

ErrorReporter::ErrorReporter() : ErrorBus(), ErrorQueue(&ErrorBus) {
  // every error logged automatically
  // errors that can be handled have other handlers bound
  ErrorBus.Subscribe<Error>(
      std::bind(&ErrorReporter::ErrorOutput, this, std::placeholders::_1));
}

// main handler for errors, unless resolution is possible and another function
// is used
void ErrorReporter::ErrorOutput(Error ErrorToReport) {
  std::string LogColour = "";
  std::string CallCode = "";

  switch (ErrorToReport.Level) {
    case ErrorLevel::TRACE:
      LogColour = WHITE;
      CallCode = "TRACE";
      break;
    case ErrorLevel::DEBUG:
      LogColour = WHITE;
      CallCode = "DEBUG";
      break;
    case ErrorLevel::INFO:
      LogColour = YELLOW;
      CallCode = "INFO";
      break;
    case ErrorLevel::WARNING:
      LogColour = ORANGE;
      CallCode = "WARNING";
      break;
    case ErrorLevel::ERR:
      LogColour = LIGHT_RED;
      CallCode = "ERROR";
      break;
    case ErrorLevel::FATAL:
      LogColour = BRIGHT_RED;
      CallCode = "FATAL";
      break;
    default:
      LogColour = WHITE;
      CallCode = "UNKNOWN";
      break;
  }

  std::cout << LogColour << "[" << CallCode << " "
            << uint32_t(ErrorToReport.Code) << "]" << RESET << " At "
            << ErrorToReport.Origin
            << ", Severity: " << uint16_t(ErrorToReport.Level)
            << " Details: " << ErrorToReport.Message << std::endl;
}

void ErrorReporter::Dispatch() { ErrorQueue.Dispatch(); }
