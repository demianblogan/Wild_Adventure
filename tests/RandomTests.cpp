#include "doctest/doctest.h"

#include "core/Random.h"

namespace
{
	constexpr int SAMPLE_COUNT = 200; // enough iterations to catch an off-by-one at either bound
}

TEST_SUITE("Random")
{
	TEST_CASE("Int stays within an inclusive range")
	{
		for (int i = 0; i < SAMPLE_COUNT; i++)
		{
			const int value = Random::Int(-5, 5);
			CHECK(value >= -5);
			CHECK(value <= 5);
		}
	}

	TEST_CASE("Int swaps a reversed range instead of misbehaving")
	{
		for (int i = 0; i < SAMPLE_COUNT; i++)
		{
			const int value = Random::Int(10, 1);
			CHECK(value >= 1);
			CHECK(value <= 10);
		}
	}

	TEST_CASE("Int with equal bounds always returns that value")
	{
		CHECK(Random::Int(7, 7) == 7);
	}

	TEST_CASE("Float stays within an inclusive range")
	{
		for (int i = 0; i < SAMPLE_COUNT; i++)
		{
			const float value = Random::Float(-1.0f, 1.0f);
			CHECK(value >= -1.0f);
			CHECK(value <= 1.0f);
		}
	}
}
