//Copyright © 2025 Henry Frodsham
#pragma once


//basic metrics to prove everything is working with input
struct InputMetric {
	int NumPressedKeys;
	float AxisY;
	float AxisX;

	InputMetric(int Pkeys, float AxY, float AxX)
		: NumPressedKeys(Pkeys)
		, AxisY(AxY)
		, AxisX(AxX){}

	bool operator==(const InputMetric& Other) const {
		return NumPressedKeys == Other.NumPressedKeys && AxisY == Other.AxisY && AxisX == Other.AxisX;
	}
};

template<>
struct std::hash<InputMetric> {
	std::size_t operator()(const InputMetric& metric) const {
		std::size_t H1 = std::hash<int>()(metric.NumPressedKeys);
		std::size_t H2 = std::hash<int>()(metric.AxisY);
		std::size_t H3 = std::hash<int>()(metric.AxisX);
		return H1 ^ (H2 << 1) ^ (H3 << 2);
	}
};