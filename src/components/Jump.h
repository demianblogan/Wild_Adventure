#pragma once

namespace ECS
{
	struct Jump
	{
		float jumpSpeed = 0.0f;   // upward speed applied on a jump
		int maxJumps = 1;         // 2 = single + one mid-air jump
		int jumpsRemaining = 0;   // refilled on the ground
		bool wantsToJump = false; // set by input on the press edge, consumed by JumpSystem
	};
}