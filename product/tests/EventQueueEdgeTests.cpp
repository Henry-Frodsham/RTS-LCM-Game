// Copyright (c) 2026 Henry Frodsham
#include <doctest/doctest.h>

#include <vector>

#include "EventBus.h"
#include "EventQueue.h"

namespace {
struct OrderedEvent {
  int Value;
  explicit OrderedEvent(int V) : Value(V) {}
};
}  // namespace

TEST_CASE("EventQueue - multiple enqueued events dispatch in FIFO order") {
  EventQueue Queue;
  EventBus Bus;
  std::vector<int> Seen;

  Bus.Subscribe<OrderedEvent>(
      [&Seen](const OrderedEvent& E) { Seen.push_back(E.Value); });

  Queue.Enqueue(OrderedEvent(1));
  Queue.Enqueue(OrderedEvent(2));
  Queue.Enqueue(OrderedEvent(3));
  Queue.Dispatch(Bus);

  REQUIRE(Seen.size() == 3);
  CHECK(Seen[0] == 1);
  CHECK(Seen[1] == 2);
  CHECK(Seen[2] == 3);
}

TEST_CASE("EventQueue - Reset clears queued events before they dispatch") {
  EventQueue Queue;
  EventBus Bus;
  bool HandlerCalled = false;

  Bus.Subscribe<OrderedEvent>(
      [&HandlerCalled](const OrderedEvent&) { HandlerCalled = true; });

  Queue.Enqueue(OrderedEvent(1));
  Queue.Reset();
  Queue.Dispatch(Bus);

  CHECK_FALSE(HandlerCalled);
}

TEST_CASE("EventQueue - Dispatch() without an assumed bus is a safe no-op") {
  EventQueue Queue;  // no bus supplied to the constructor
  Queue.Enqueue(OrderedEvent(1));
  CHECK_NOTHROW(Queue.Dispatch());
}

TEST_CASE("EventQueue - Dispatch() with the default-constructed bus reaches "
         "subscribed handlers") {
  EventBus Bus;
  EventQueue Queue(&Bus);
  int SeenValue = 0;

  Bus.Subscribe<OrderedEvent>(
      [&SeenValue](const OrderedEvent& E) { SeenValue = E.Value; });

  Queue.Enqueue(OrderedEvent(42));
  Queue.Dispatch();

  CHECK(SeenValue == 42);
}
