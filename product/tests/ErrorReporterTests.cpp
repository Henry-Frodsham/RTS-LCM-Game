#include <doctest/doctest.h>

#include "ErrorReporter.h"

TEST_CASE("ErrorReporter - creation") { CHECK_NOTHROW(ErrorReporter Obj()); }

TEST_CASE("ErrorReporter - error handler behaviour") {
  ErrorReporter* MockReporter = new ErrorReporter();
  bool errorOccurred = false;

  DeadDeviceIdError MockError = DeadDeviceIdError{nullptr, 0};
  std::function<void(const DeadDeviceIdError)> Handler =
      [&errorOccurred](const DeadDeviceIdError& error) {
        errorOccurred = true;
      };

  MockReporter->AddHandler<DeadDeviceIdError>(Handler);
  MockReporter->EnqueueError(MockError);
  MockReporter->Dispatch();
  CHECK(errorOccurred);
}
