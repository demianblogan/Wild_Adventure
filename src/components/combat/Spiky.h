#pragma once

namespace ECS
{
	// Marks an enemy as armored right now: EnemySystem treats a stomp on it like any
	// other contact, so it hurts the player and survives. The turtle adds and removes
	// this as its spikes come out and retract.
	struct Spiky
	{
	};
}
