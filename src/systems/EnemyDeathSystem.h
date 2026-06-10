#pragma once

#include "core/ecs/Registry.h"

namespace ECS
{
	class EnemyDeathSystem
	{
	public:
		EnemyDeathSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		Registry& registry;
	};
}
