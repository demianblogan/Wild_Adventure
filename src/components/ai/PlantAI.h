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
		bool  hasFiredBullet = false; // true once the bullet is spawned for the current attack cycle

		static constexpr float ShootInterval  = 0.25f;  // seconds between shots
		static constexpr float SightRange     = 220.0f; // pixels
		static constexpr float SightTolerance = 20.0f;  // Y delta allowed for "same line"
		static constexpr float BulletSpeed    = 200.0f; // faster than the trunk's bullets
		static constexpr int   FireFrame      = 4;      // 5th attack frame (0-based) the bullet leaves on
		static constexpr float BulletHeight   = 16.0f;  // pixels above the plant's base the bullet exits at
	};
}