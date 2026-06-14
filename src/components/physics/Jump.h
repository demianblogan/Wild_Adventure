#pragma once

namespace ECS
{
	struct Jump
	{
		float jumpSpeed = 0.0f;
		int maxJumps = 1;
		int jumpsRemaining = 0;
		bool wantsToJump = false;

		float wallJumpPushX = 0.0f; // horizontal push away from a wall on a wall jump
		float lockTimer = 0.0f;     // runtime: input ignores horizontal control while > 0
	};
}