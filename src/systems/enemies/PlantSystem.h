#pragma once

#include "components/Collider.h"
#include "components/Transform.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	class PlantSystem
	{
	public:
		PlantSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		void SpawnBullet(const Transform& transform, const Collider& collider, int direction);

		Registry& registry;
	};
}
