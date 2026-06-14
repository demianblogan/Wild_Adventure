#pragma once

namespace ECS
{
	struct Patrol
	{
		enum class Axis { Horizontal, Vertical };

		Axis axis = Axis::Horizontal;
		float min = 0.0f;     // lower bound on the patrol axis, in pixels
		float max = 0.0f;     // upper bound on the patrol axis, in pixels
		float speed = 0.0f;   // pixels per second
		int direction = 1;    // +1 toward max, -1 toward min
	};
}