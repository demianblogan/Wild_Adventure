#pragma once

namespace ECS
{
	// Tag: the entity is held in place - no input, no gravity/physics, not rendered.
	// Used while the hero materializes (appear) or vanishes (disappear).
	struct Frozen
	{};
}