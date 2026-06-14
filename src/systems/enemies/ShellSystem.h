#pragma once

#include "core/ecs/Registry.h"

namespace Audio { class Mixer; }

namespace ECS
{
	// Drives the snail's shell: it rests until the player touches it, then rolls
	// along the ground, reversing off walls, hurting the player on side contact,
	// and dying when stomped from above.
	class ShellSystem
	{
	public:
		ShellSystem(Registry& registry, Audio::Mixer& mixer);
		void Update(float deltaTime);

	private:
		Registry&     registry;
		Audio::Mixer& mixer;
	};
}
