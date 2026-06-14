#pragma once

namespace ECS
{
	// Marks a trap that launches the player up via its Solid's bounceSpeed.
	// PhysicsSystem raises wasBounced on the bounce; TrampolineSystem consumes
	// it to play the one-shot Jump animation and return to Idle.
	struct Trampoline
	{
		bool wasBounced = false;
	};
}
