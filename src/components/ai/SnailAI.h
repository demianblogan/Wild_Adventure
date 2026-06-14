#pragma once

namespace ECS
{
	// Marker for the alive snail. Movement is handled by GroundPatrol; SnailSystem
	// only drives the stomp-death sequence (Hit animation -> split into body + shell).
	struct SnailAI
	{
	};
}
