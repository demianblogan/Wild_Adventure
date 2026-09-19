#pragma once

namespace ECS
{
	struct TrunkAI
	{
		enum class State { Patrolling, Shooting };

		State state = State::Patrolling;
		float shootCooldown = 0.0f;
		bool  hasFiredBullet = false; // true once bullet is spawned for the current attack cycle

		static constexpr float ShootInterval = 0.4f;   // seconds between shots
		static constexpr float SightRange = 200.0f;    // pixels
		static constexpr float SightTolerance = 20.0f; // Y delta allowed for "same line"
		static constexpr float BulletSpeed = 120.0f;
		static constexpr int   FireFrame = 7;          // attack frame (0-based) the bullet leaves on
	};
}
