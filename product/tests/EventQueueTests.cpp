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
	Obj.enqueue(std::make_unique<std::any>(TestEvent()));
	Obj.dispatch(Obj2);
	CHECK(HandlerCalled);
}

TEST_CASE("EventQueue - homogenous event delay") {
	EventQueue Obj;
	EventBus Obj2;
	std::vector<std::chrono::system_clock::time_point> EventTimeStamps;
	Obj2.Subscribe<TestEvent>([&EventTimeStamps](const TestEvent& event) {
		EventTimeStamps.push_back(event.TimeStamp);
		});
	Obj.enqueue(std::make_unique<std::any>(TestEvent()));
	Obj.enqueue(std::make_unique<std::any>(TestEvent()));
	Obj.dispatch(Obj2);

	// race condition if they match
	// todo : only delay events accessing the same resource and not just homogenous events
	if (EventTimeStamps.size() == 2) {
		CHECK(EventTimeStamps[0] != EventTimeStamps[1]);
	}
	else {
		CHECK(false);
	}
}

