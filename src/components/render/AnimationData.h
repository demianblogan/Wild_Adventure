#pragma once

#include <string>

namespace ECS
{
	struct AnimationData
	{
		std::string textureName;
		int frameCount = 1;
		float frameDuration = 0.1f;
		bool isLooping = true;
		bool isReversed = false;

		// Dust emitted under the entity each time particleFrame finishes
		// (e.g. the blue bird's wing flap). -1 disables the emission.
		std::string particlePreset;
		int particleFrame = -1;
	};
}