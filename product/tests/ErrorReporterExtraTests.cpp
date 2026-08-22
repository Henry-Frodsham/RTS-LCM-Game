// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "ErrorReporter.h"

namespace {
// a second concrete derived error, distinct from DeadDeviceIdError, purely
// to prove handlers stay isolated per concrete type
struct TestOnlyError : Error {
  explicit TestOnlyError(std::string CustomMessage)
      : Error(ErrorDetail::CreateError(ErrorCode::UNRECOGNISED,
                                       CustomMessage)) {}
};
}  // namespace

TEST_CASE("ErrorReporter - EnqueueError on a derived type does not throw") {
  ErrorReporter Reporter;
  CHECK_NOTHROW(Reporter.EnqueueError(DeadDeviceIdError(nullptr, 1)));
  CHECK_NOTHROW(Reporter.Dispatch());
}

TEST_CASE("ErrorReporter - a handler for one derived error type does not "
         "fire for a different derived error type") {
  ErrorReporter Reporter;
  bool DeadDeviceHandlerCalled = false;
  bool TestOnlyHandlerCalled = false;

  Reporter.AddHandler<DeadDeviceIdError>(
      [&DeadDeviceHandlerCalled](const DeadDeviceIdError&) {
        DeadDeviceHandlerCalled = true;
      });
  Reporter.AddHandler<TestOnlyError>(
      [&TestOnlyHandlerCalled](const TestOnlyError&) {
        TestOnlyHandlerCalled = true;
      });

  Reporter.EnqueueError(TestOnlyError("only this one"));
  Reporter.Dispatch();

  CHECK(TestOnlyHandlerCalled);
  CHECK_FALSE(DeadDeviceHandlerCalled);
}

TEST_CASE("ErrorReporter - dispatching with nothing newly queued does not "
         "re-invoke a previous handler") {
  ErrorReporter Reporter;
  int Calls = 0;

  Reporter.AddHandler<TestOnlyError>(
      [&Calls](const TestOnlyError&) { Calls += 1; });

  Reporter.EnqueueError(TestOnlyError("first"));
  Reporter.Dispatch();
  CHECK(Calls == 1);

  Reporter.Dispatch();  // nothing new queued
  CHECK(Calls == 1);
}
