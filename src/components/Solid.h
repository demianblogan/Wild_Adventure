#pragma once

namespace ECS
{
	// A solid axis-aligned box (feet-anchored, like sprites). The player collides with it
	// as if it were terrain. bounceSpeed > 0 makes the top surface a trampoline.
	struct Solid
	{
		float width = 0.0f;
		float height = 0.0f;
		float bounceSpeed = 0.0f;
	};
}