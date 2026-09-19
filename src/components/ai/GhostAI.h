#pragma once

namespace ECS
{
	// A ground patroller that fades out and back in on a fixed cycle. It keeps moving
	// while invisible (so it vanishes in one spot and reappears in another) and cannot
	// hit or be hit during the invisible phase. Patrol movement comes from GroundPatrol;
	// GhostSystem owns the visibility cycle, animation, and trailing particles.
	struct GhostAI
	{
		enum class Phase { Visible, Disappearing, Invisible, Appearing };

		Phase phase = Phase::Visible;
		float phaseTimer = VisibleDuration;
		float particleTimer = 0.0f;

		static constexpr float VisibleDuration = 1.0f;   // seconds shown before fading out
		static constexpr float InvisibleDuration = 2.0f; // seconds hidden before fading back in
		static constexpr float ParticleInterval = 0.12f; // gap between trailing wisps while visible
	};
}