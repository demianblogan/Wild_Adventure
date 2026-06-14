#pragma once

#include "core/ecs/Registry.h"

class DataLoader;

namespace ECS
{
	// Drives the snail's stomp death: once EnemySystem tags a stomped snail, this
	// system holds it still through the Hit animation, then replaces it with a
	// flying body (dies like other enemies) and a shell that stays on the ground.
	class SnailSystem
	{
	public:
		SnailSystem(Registry& registry, DataLoader& loader);
		void Update(float deltaTime);

	private:
		void SpawnShell(float x, float y, bool faceRight);
		void SpawnBody(float x, float y);

		Registry&   registry;
		DataLoader& loader;
	};
}
