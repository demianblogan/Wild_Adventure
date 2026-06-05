#pragma once

namespace ECS
{
	// Marker: the entity is playing its final (non-looping) animation and is removed
	// once that animation reaches its last frame.
	struct Despawning
	{};
}