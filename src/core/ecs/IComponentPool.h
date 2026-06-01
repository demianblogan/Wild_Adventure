#pragma once

#include "core/ecs/Entity.h"

namespace ECS
{
	class IComponentPool
	{
	public:
		virtual ~IComponentPool() = default;

		virtual void RemoveFrom(Entity entity) = 0;
		virtual bool Has(Entity entity) const = 0;
	};
}