#pragma once

namespace ECS
{
	struct TrunkAI
	{
		enum class State { Patrolling, Shooting };

		State state          = State::Patrolling;
		float shootCooldown  = 0.0f;
		bool  bulletFired    = false; // true once bullet is spawned for the current attack cycle

		static constexpr float SHOOT_INTERVAL  = 0.4f;  // seconds between shots
		static constexpr float SIGHT_RANGE     = 200.0f; // pixels
		static constexpr float SIGHT_TOLERANCE = 20.0f;  // Y delta allowed for "same line"
		static constexpr float BULLET_SPEED    = 120.0f;
	};
}
