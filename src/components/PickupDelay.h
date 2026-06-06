#pragma once

namespace ECS
{
	struct PickupDelay
	{
		float timer = 0.0f; // not collectible until this reaches zero
	};
}