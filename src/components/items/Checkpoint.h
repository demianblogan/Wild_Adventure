#pragma once

namespace ECS
{
	// A respawn checkpoint. Once touched it stays activated (flag waving) and can no
	// longer be re-triggered.
	struct Checkpoint
	{
		bool activated = false;
	};
}