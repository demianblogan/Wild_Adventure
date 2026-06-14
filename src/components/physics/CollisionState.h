#pragma once

namespace ECS
{
	struct CollisionState
	{
		bool isOnGround = false;
		bool isOnCeiling = false;
		bool isOnWall = false;
		int wallDirection = 0; // -1 wall on the left, +1 wall on the right, 0 none
	};
}