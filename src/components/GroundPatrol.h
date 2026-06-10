#pragma once

namespace ECS
{
	struct GroundPatrol
	{
		enum class State { Patrolling, TurningIdle };

		State state      = State::Patrolling;
		int   direction  = -1;
		float speed      = 30.0f;
		float stateTimer = 0.0f;

		static constexpr float TURN_IDLE_DURATION = 0.5f;
	};
}
