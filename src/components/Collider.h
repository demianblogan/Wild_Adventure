#pragma once

namespace ECS
{
	// Axis-aligned collision box, anchored to the feet like the sprite.
	// This is the collision size, not the frame size.
	struct Collider
	{
		float width = 0.0f;
		float height = 0.0f;
	};
}