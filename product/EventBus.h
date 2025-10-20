//Copyright © 2025 Henry Frodsham
#pragma once
#include <unordered_map>
#include <vector>
#include <any>
#include <typeindex>
#include <functional>

// notifies stored handlers when an event is published
class EventBus {
private:
	std::unordered_map<std::type_index, std::vector<std::function<void(std::any)>>> handlers;

public:
	EventBus() {}
	~EventBus() {}

	template<typename EventType>
	void Subscribe(std::function<void(const EventType&)> handler) {}

	template<typename EventType>
	void Publish(const EventType& event) {}
};