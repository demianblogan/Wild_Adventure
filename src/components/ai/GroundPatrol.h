#pragma once

#include <string>

namespace ECS
{
	struct GroundPatrol
	{
		enum class State { Patrolling, TurningIdle };

		State state      = State::Patrolling;
		int   direction  = -1;
		float speed      = 30.0f;
		float stateTimer = 0.0f;
		bool  paused     = false; // set by AI systems (e.g. TrunkAI) to suspend patrol
		float dustTimer  = 0.0f;  // runtime: countdown to the next run-dust puff

		float turnIdleDuration = TURN_IDLE_DURATION; // how long to pause when turning (per-entity)

		// Animation states to play while moving / paused (per-entity, e.g. the snail walks).
		std::string moveAnim = "Run";
		std::string idleAnim = "Idle";

		bool emitsDust       = true; // leave a trail of run dust on the floor
		bool managesAnimation = true; // false when another system owns the animation (e.g. the ghost)

		static constexpr float TURN_IDLE_DURATION = 0.5f;
		static constexpr float DUST_SPACING       = 14.0f; // distance (px) between dust puffs
	};
}
