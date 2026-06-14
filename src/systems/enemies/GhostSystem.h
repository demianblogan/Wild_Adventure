#pragma once

#include "core/ecs/Registry.h"

class ParticleSystem;

namespace ECS
{
	// Drives the ghost's visibility cycle: visible (Idle) -> Disappear animation ->
	// invisible (hidden, no collisions, still patrolling) -> Appear animation -> repeat.
	// Also trails ghost-wisp particles from its back while it is visible.
	class GhostSystem
	{
	public:
		GhostSystem(Registry& registry, ParticleSystem& particles);
		void Update(float deltaTime);

	private:
		Registry&       registry;
		ParticleSystem& particles;
	};
}
