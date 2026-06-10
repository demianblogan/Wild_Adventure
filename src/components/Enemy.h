#pragma once

namespace ECS
{
	struct Enemy
	{
		float spawnX = 0.0f; // recorded post-load; used to match enemies in checkpoint snapshots
		float spawnY = 0.0f;
	};
}
