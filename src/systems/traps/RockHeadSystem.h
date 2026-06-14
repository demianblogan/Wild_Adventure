#pragma once

#include "core/ecs/Registry.h"
#include "tilemap/Tilemap.h"

namespace ECS
{
	// Drives the rock-head platform: ramps its speed each pass, reverses when PhysicsSystem
	// reports it hit the terrain (playing the matching directional hit animation), blinks
	// while idling, carries the player riding on top, and crushes the player pinned against
	// a wall. Runs after PhysicsSystem (which does the actual moving and collision).
	class RockHeadSystem
	{
	public:
		RockHeadSystem(Registry& registry, const Tilemap& tilemap);
		void Update(float deltaTime);

	private:
		Registry&      registry;
		const Tilemap& tilemap;
	};
}
