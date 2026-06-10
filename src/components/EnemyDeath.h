#pragma once

namespace ECS
{
	struct EnemyDeath
	{
		enum class State { DeathPause, DeathFalling };

		State state      = State::DeathPause;
		float stateTimer = DEATH_PAUSE_DURATION; // initialized to full duration when added

		static constexpr float DEATH_PAUSE_DURATION = 0.1f;
		static constexpr float DEATH_BOUNCE_SPEED   = 150.0f;
	};
}
