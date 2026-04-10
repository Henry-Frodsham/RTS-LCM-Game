// Copyright (c) 2025 Henry Frodsham
#include <doctest/doctest.h>

#include <string>

#include "EventBus.h"

struct TestEvent {};

void TestHandler(TestEvent) {}

TEST_CASE("EventBus - constructor") { CHECK_NOTHROW(EventBus obj()); }

TEST_CASE("EventBus - subscribe and publish") {
  EventBus Obj{};
  bool HandlerCalled = false;

  Obj.Subscribe<TestEvent>(
      [&HandlerCalled](const TestEvent& event) { HandlerCalled = true; });

  Obj.Publish(TestEvent());

  CHECK(HandlerCalled);
}

TEST_CASE("EventBus - singular handler call from publish") {
  EventBus Obj{};
  int NumCalls = 0;

  Obj.Subscribe<TestEvent>(
      [&NumCalls](const TestEvent& event) { NumCalls += 1; });

  Obj.Publish(TestEvent());

  CHECK(NumCalls == 1); // NOLINT [readability/check]
}
