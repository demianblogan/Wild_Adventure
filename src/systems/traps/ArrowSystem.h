#pragma once

#include "core/ecs/Registry.h"

namespace Audio { class Mixer; }

namespace Haptics
{
	class GamepadHaptics;
}

namespace ECS
{
	// Drives the arrow air booster: when the player overlaps one, it flings them
	// straight up, plays its one-shot Hit animation, and then despawns.
	class ArrowSystem
	{
	public:
		ArrowSystem(Registry& registry, Audio::Mixer& mixer, Haptics::GamepadHaptics& gamepadHaptics);
		void Update();

	private:
		Registry&     registry;
		Audio::Mixer& mixer;
		Haptics::GamepadHaptics& gamepadHaptics;
	};
}
