#pragma once

namespace ECS
{
	struct ChickenAI
	{
		enum class State { Idle, Chasing };

		State state          = State::Idle;
		float visionWidth    = 160.0f; // vision rectangle centered on the chicken, in pixels
		float visionHeight   = 80.0f;
		float speed          = 135.0f; // slightly faster than the player's 120
		float loseSightTimer = 0.0f;   // runtime: keeps the chase alive briefly after losing sight
		float dustTimer      = 0.0f;   // runtime: countdown to the next run-dust puff

		static constexpr float LOSE_SIGHT_DELAY = 0.5f;  // seconds of chasing after sight is lost
		static constexpr float STOP_DISTANCE    = 2.0f;  // X delta where the chicken stands instead of jittering
		static constexpr float DUST_SPACING     = 14.0f; // distance (px) between dust puffs
	};
}
