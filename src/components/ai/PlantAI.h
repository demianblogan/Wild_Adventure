#pragma once

namespace ECS
{
	// A stationary enemy that spits bullets when the player lines up in front of it.
	// The plant never turns; the facing direction comes from its Facing component
	// (set per-instance via the "direction" property in Tiled).
	struct PlantAI
	{
		enum class State { Idle, Attacking };

		State state         = State::Idle;
		float shootCooldown = 0.0f;
		bool  bulletFired   = false; // true once the bullet is spawned for the current attack cycle

		static constexpr float SHOOT_INTERVAL  = 0.25f;  // seconds between shots
		static constexpr float SIGHT_RANGE     = 220.0f; // pixels
		static constexpr float SIGHT_TOLERANCE = 20.0f;  // Y delta allowed for "same line"
		static constexpr float BULLET_SPEED    = 200.0f; // faster than the trunk's bullets
		static constexpr int   FIRE_FRAME      = 4;      // 5th attack frame (0-based) the bullet leaves on
		static constexpr float BULLET_HEIGHT   = 16.0f;  // pixels above the plant's base the bullet exits at
	};
}
