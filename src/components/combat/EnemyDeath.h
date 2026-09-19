#pragma once

namespace ECS
{
	struct EnemyDeath
	{
		enum class State { DeathPause, DeathFalling };

		State state = State::DeathPause;
		float stateTimer = DeathPauseDuration; // initialized to full duration when added

		static constexpr float DeathPauseDuration = 0.1f;
		static constexpr float DeathBounceSpeed = 150.0f;
		static constexpr float FallGravity = 800.0f; // for flyers that patrol with zero gravity
		static constexpr float MaxFallSpeed = 500.0f;
		static constexpr float SpinBaseSpeed = 270.0f;     // degrees/second, before the random extra
		static constexpr float SpinSpeedVariance = 90.0f;  // random extra added on top of the base
	};
}