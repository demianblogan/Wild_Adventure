#pragma once

namespace ECS
{
	// The snail's shell left behind after it is stomped. It rests until the player
	// touches it, then rolls along the ground (Mario-koopa style) until it hits a
	// wall (reverses, without stopping) or is stomped from above (dies). The wall
	// impact is purely an animation overlay played while it keeps rolling.
	struct Shell
	{
		enum class State { Resting, Rolling };

		State state     = State::Resting;
		int   direction = -1;     // roll direction once kicked: -1 left, +1 right
		float speed     = 160.0f; // rolling speed, px/s
		float kickGrace = 0.0f;   // seconds before it can be kicked (avoids an instant kick on spawn)

		// Right after a kick the player still overlaps the shell; it must not hurt them
		// until they have stepped clear, so pushing it never "rolls into" the kicker.
		bool harmlessToKicker = false;
	};
}
