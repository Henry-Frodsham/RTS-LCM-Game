// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include "EventBus.h"

namespace {
struct EventA {};
struct EventB {};
}  // namespace

TEST_CASE("EventBus - publishing an event with no subscribers does not "
         "throw") {
  EventBus Bus;
  CHECK_NOTHROW(Bus.Publish(EventA{}));
}

TEST_CASE("EventBus - multiple handlers for the same event type all run") {
  EventBus Bus;
  int Calls = 0;

  Bus.Subscribe<EventA>([&Calls](const EventA&) { Calls += 1; });
  Bus.Subscribe<EventA>([&Calls](const EventA&) { Calls += 1; });

  Bus.Publish(EventA{});

  CHECK(Calls == 2);
}

TEST_CASE("EventBus - handlers only fire for their own event type") {
  EventBus Bus;
  bool AHandlerCalled = false;
  bool BHandlerCalled = false;

  Bus.Subscribe<EventA>([&AHandlerCalled](const EventA&) {
    AHandlerCalled = true;
  });
  Bus.Subscribe<EventB>([&BHandlerCalled](const EventB&) {
    BHandlerCalled = true;
  });

  Bus.Publish(EventA{});

  CHECK(AHandlerCalled);
  CHECK_FALSE(BHandlerCalled);
}
