#pragma once

namespace ECS
{
	// Temporarily not rendered and not interactive (e.g. the ghost during its invisible
	// phase). RenderSystem and EnemySystem skip a Hidden entity, but movement systems
	// keep running, so a hidden enemy still patrols.
	struct Hidden
	{
	};
}
