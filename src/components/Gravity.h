#pragma once

namespace ECS
{
	struct Gravity
	{
		float acceleration = 0.0f;  // pixels per second^2
		float maxFallSpeed = 0.0f;  // terminal velocity: also keeps a fast fall from skipping tiles
	};
}