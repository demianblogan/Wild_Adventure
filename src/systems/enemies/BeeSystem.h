#pragma once

#include "components/Transform.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	class BeeSystem
	{
	public:
		BeeSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		void SpawnBullet(const Transform& transform);

		Registry& registry;
	};
}
