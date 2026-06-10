#pragma once

#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

namespace ECS
{
	class BulletSystem
	{
	public:
		BulletSystem(Registry& registry, const Tilemap& tilemap);
		void Update(float deltaTime);

	private:
		void SpawnPieces(float x, float y, int bulletDirection);

		Registry&      registry;
		const Tilemap& tilemap;
	};
}
