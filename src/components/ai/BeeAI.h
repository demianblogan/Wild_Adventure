#pragma once

namespace ECS
{
	// A flying enemy that traces a horizontal figure-8 around its spawn point and
	// spits bullets straight down while the player is horizontally near. The bee
	// never turns (it always faces the camera), so it has no Facing component.
	struct BeeAI
	{
		enum class State { Flying, Attacking };

		State state = State::Flying;

		// Figure-8 flight path (Lissajous 1:2) around the spawn anchor.
		float width  = 64.0f; // full horizontal extent of the figure-8 (set in prefab)
		float height = 32.0f; // full vertical extent of the figure-8 (set in prefab)
		float speed  = 2.0f;  // angular speed along the path, radians/second

		bool  initialized = false; // anchor is captured on the first update
		float anchorX     = 0.0f;
		float anchorY     = 0.0f;
		float time        = 0.0f;  // accumulated path time

		// Attack cycle.
		float shootCooldown = 0.0f;
		bool  bulletFired   = false; // true once the bullet is spawned for the current cycle

		static constexpr float SHOOT_INTERVAL      = 0.25f;  // seconds between shots
		static constexpr float ATTACK_RANGE_FACTOR = 3.0f;   // x the bee's collider width
		static constexpr float BULLET_SPEED        = 220.0f; // downward bullet speed
		static constexpr int   FIRE_FRAME          = 4;      // 5th attack frame (0-based)
	};
}
