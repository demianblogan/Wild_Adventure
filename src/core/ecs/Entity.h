#pragma once

#include <cstdint>

namespace ECS
{
	// A plain, ever-increasing id: Registry::CreateEntity never reuses one, so
	// there is no generation counter to go with it. That is a deliberate
	// trade-off, not an oversight -- see the comment on Registry::nextEntity.
	using Entity = std::uint32_t;

	constexpr Entity InvalidEntity = ~Entity{ 0 };
}