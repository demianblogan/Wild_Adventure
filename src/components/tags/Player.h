#pragma once

namespace ECS
{
	struct Player
	{
		float moveSpeed = 0.0f;
		float acceleration = 0.0f; // how fast speed builds up while a key is held
		float deceleration = 0.0f; // how fast it bleeds off when no key is held
	};
}