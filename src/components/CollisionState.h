#pragma once

namespace ECS
{
	// Written by the physics system, read by animation (and later by jump logic).
	struct CollisionState
	{
		bool isOnGround = false;
	};
}