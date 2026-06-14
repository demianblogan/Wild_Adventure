#pragma once

namespace ECS
{
	// Opt-in tag: when an enemy with this tag is stomped, EnemySystem marks it with
	// a Stomped tag instead of the usual EnemyDeath, leaving the death sequence to a
	// dedicated system (e.g. SnailSystem turning the snail into a body and a shell).
	struct StompCustomDeath
	{
	};
}
