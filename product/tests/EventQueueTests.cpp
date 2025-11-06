//Copyright © 2025 Henry Frodsham
#include "EventQueue.h"
#include "EventBus.h"
#include <chrono>
#include <vector>
#include <doctest/doctest.h>

struct TestEvent {
	std::string data = "test";
	std::chrono::system_clock::time_point TimeStamp;

	TestEvent() : TimeStamp(std::chrono::system_clock::now()) {}
};

TEST_CASE("EventQueue - constructor") {
	CHECK_NOTHROW(EventQueue Obj());
}

TEST_CASE("EventQueue - event schedule and dispatch") {
	EventQueue Obj;
	EventBus Obj2;
	bool HandlerCalled = false;
	Obj2.Subscribe<TestEvent>([&HandlerCalled](const TestEvent& event) {
	HandlerCalled = true;
		});
	Obj.Enqueue(TestEvent());
	Obj.Dispatch(Obj2);
	CHECK(HandlerCalled);
}


