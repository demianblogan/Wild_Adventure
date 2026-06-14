#pragma once

namespace ECS
{
	// Set by EnemySystem when a StompCustomDeath enemy is stomped. Shared systems
	// (EnemySystem, GroundPatrolSystem) skip a Stomped entity so the enemy's own
	// system can run its bespoke death sequence undisturbed.
	struct Stomped
	{
	};
}
