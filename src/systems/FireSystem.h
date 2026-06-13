#pragma once

#include "core/ecs/Registry.h"

namespace ECS
{
	// Drives the fire-plate trap: Off -> (player steps on) Hit warm-up -> On (flame,
	// harmful) for a couple of seconds -> Off. While On it carries a Hazard so the
	// shared DamageSystem applies the burn; FireSystem just toggles the state and tag.
	class FireSystem
	{
	public:
		FireSystem(Registry& registry);
		void Update(float deltaTime);

	private:
		Registry& registry;
	};
}
