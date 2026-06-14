#pragma once

#include <random>

class Random
{
public:
	Random() = delete;

	[[nodiscard]] static int Int(int min, int max);
	[[nodiscard]] static float Float(float min, float max);

	static std::mt19937_64& Engine();
};