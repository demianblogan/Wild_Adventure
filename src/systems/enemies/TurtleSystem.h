#pragma once

#include "core/ecs/Registry.h"

namespace ECS
{
	// Drives the turtle's spike cycle: Idle2 (spikes in, vulnerable) -> Spikes out
	// (now armored) -> Idle1 (spikes out) -> Spikes in -> vulnerable again. Toggles the
	// Spiky tag so EnemySystem knows when a stomp should kill it versus hurt the player.
	class TurtleSystem
	{
	public:
		TurtleSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		Registry& registry;
	};
}
