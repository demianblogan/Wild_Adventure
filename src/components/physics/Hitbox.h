#pragma once

namespace ECS
{
	// Damage area, anchored to the feet like the sprite. Kept separate from Collider
	// so hazards aren't treated as solid bodies by movement/physics.
	struct Hitbox
	{
		float width = 0.0f;
		float height = 0.0f;
	};
}