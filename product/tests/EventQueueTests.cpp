#include "EventQueue.h"
#include "EventBus.h"
#include <doctest/doctest.h>

struct TestEvent {
	std::string data = "test";
};

TEST_CASE("EventQueue - constructor") {
	CHECK_NOTHROW(EventQueue Obj);
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

