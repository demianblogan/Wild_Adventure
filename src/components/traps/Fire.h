#pragma once

namespace ECS
{
	// A floor plate trap. It is harmless (Off) until the player steps on it, then plays
	// a short warm-up (Hit) before bursting into flame (On), which hurts the player for
	// a couple of seconds. FireSystem drives the cycle; the burn itself is dealt by the
	// shared DamageSystem via a Hazard that FireSystem adds only while On.
	struct Fire
	{
		enum class State { Off, Activating, On };

		State state   = State::Off;
		float onTimer = 0.0f; // counts down the On phase

		static constexpr float ON_DURATION = 2.0f; // seconds the flame stays up
		static constexpr int   BURN_DAMAGE = 1;    // damage dealt while On
	};
}
