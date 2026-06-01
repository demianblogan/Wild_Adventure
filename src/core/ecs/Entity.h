#pragma once

#include <cstdint>

namespace ECS
{
	using Entity = std::uint32_t;

	constexpr Entity INVALID_ENTITY = ~Entity{ 0 };
}