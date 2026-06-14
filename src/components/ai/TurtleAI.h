#pragma once

namespace ECS
{
	// A stationary enemy that cycles its spikes in and out. It can only be stomped to
	// death while the spikes are in (Safe); while they are out it is armored (tagged
	// Spiky) so a stomp hurts the player instead. TurtleSystem drives the cycle.
	struct TurtleAI
	{
		enum class Phase { Safe, SpikesEmerging, Spiked, SpikesRetracting };

		Phase phase      = Phase::Safe;
		float phaseTimer = SAFE_DURATION;

		static constexpr float SAFE_DURATION   = 0.5f; // Idle2, spikes in: short window to stomp it
		static constexpr float SPIKED_DURATION = 2.0f; // Idle1, spikes out: armored for longer
	};
}
