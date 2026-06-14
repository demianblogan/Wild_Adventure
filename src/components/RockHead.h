#pragma once

namespace ECS
{
	// A square stone platform that flies back and forth along one axis until it hits the
	// terrain, pauses briefly, then reverses — forever. It accelerates quickly from a stop
	// at the start of each pass. As a Solid it carries the player like a lift, and it can
	// crush the player against a wall. PhysicsSystem moves it and reports the wall it hit;
	// RockHeadSystem owns the speed ramp, bouncing, animations, carrying and crushing.
	struct RockHead
	{
		enum class Axis  { Horizontal, Vertical };
		enum class State { Moving, Stopped };

		Axis  axis         = Axis::Horizontal;
		int   direction    = 1;       // along the axis: +1 right/down, -1 left/up
		float maxSpeed     = 500.0f;  // top speed cap (set in the prefab)
		float acceleration = 450.0f;  // keeps building speed across the whole pass
		float speed        = 0.0f;    // current ramped speed
		State state        = State::Moving;
		float blinkTimer   = BLINK_INTERVAL;

		static constexpr float BLINK_INTERVAL = 3.0f; // how often the idle blink plays while moving
	};
}
