#pragma once

#include "components/physics/Collider.h"
#include "components/physics/Transform.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	class TrunkSystem
	{
	public:
		TrunkSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		void SpawnBullet(const Transform& transform, const Collider& collider, int direction);

		Registry& registry;
	};
}
