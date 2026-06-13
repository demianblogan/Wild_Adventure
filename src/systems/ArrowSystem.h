#pragma once

#include "core/ecs/Registry.h"

namespace Audio { class Mixer; }

namespace ECS
{
	// Drives the arrow air booster: when the player overlaps one, it flings them
	// straight up, plays its one-shot Hit animation, and then despawns.
	class ArrowSystem
	{
	public:
		ArrowSystem(Registry& registry, Audio::Mixer& mixer);
		void Update();

	private:
		Registry&     registry;
		Audio::Mixer& mixer;
	};
}
