//Copyright © 2025 Henry Frodsham
#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <any>
#include <typeindex>
#include <functional>
#include <mutex>

// notifies stored handlers when an event is published, instant and without race condition checking
class EventBus {
private:
	std::unordered_map<std::type_index, std::vector<std::function<void(std::any)>>> Handlers;
	mutable std::mutex BusMutex;

public:
	EventBus() = default;
	~EventBus() {}

	//template functions must be written in a header, thus keeping here
	template<typename EventType>
	void Subscribe(std::function<void(const EventType&)> Handler) {
		std::lock_guard<std::mutex> Lock(BusMutex);

		std::type_index TypeIndex = std::type_index(typeid(std::decay_t<EventType>));
		auto WrappedHandler = [Handler](std::any EventData) {
			const EventType& TypedEvent = std::any_cast<const EventType&>(EventData);
			Handler(TypedEvent);
			};
		Handlers[TypeIndex].push_back(WrappedHandler);
	}

	template<typename EventType>
	void Publish(const EventType& Event) {
		std::lock_guard<std::mutex> Lock(BusMutex);
		std::type_index TypeIndex = std::type_index(typeid(std::decay_t<EventType>));
		auto It = Handlers.find(TypeIndex);
		if (It != Handlers.end()) {
			std::any EventData = Event;
			for (auto& Handler : It->second) {
				Handler(EventData);
			}
		}
	}
};