#pragma once

#include <cstddef>

namespace ECS
{
	inline std::size_t nextComponentTypeId = 0;

	template <typename T>
	std::size_t GetComponentTypeId()
	{
		static const std::size_t id = nextComponentTypeId++;
		return id;
	}
}