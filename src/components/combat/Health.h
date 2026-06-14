#pragma once

namespace ECS
{
	struct Health
	{
		int current = 0;
		int maximum = 0;

		float invulnerabilityDuration = 0.0f; // data
		float invulnerabilityTimer = 0.0f;    // runtime: counts down after a hit (blinking)

		float hitStunDuration = 0.0f;         // data: control locked + Hit animation plays
		float hitStunTimer = 0.0f;            // runtime

		float knockbackSpeed = 0.0f;          // data: push strength on a hit
	};
}