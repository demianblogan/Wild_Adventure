#pragma once

#include "components/tags/Player.h"
#include "core/ecs/Entity.h"
#include "core/ecs/Registry.h"

namespace ECS
{
	// The single player entity, or INVALID_ENTITY when none exists (e.g. the menu
	// backdrop runs a level without a player). Systems that react to the player
	// share this instead of re-rolling the same one-off ForEach.
	inline Entity FindPlayer(Registry& registry)
	{
		Entity player = INVALID_ENTITY;
		registry.ForEach<Player>([&](Entity entity, Player&) { player = entity; });
		return player;
	}
}
