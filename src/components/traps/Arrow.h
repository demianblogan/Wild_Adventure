#pragma once

namespace ECS
{
	// An air booster: on overlap it launches the player straight up (like a trampoline
	// but with no solid body), plays its one-shot Hit animation, then vanishes for good.
	struct Arrow
	{
		float boostSpeed = 700.0f;
		bool  hasTriggered = false; // true once used; it plays Hit then despawns
	};
}