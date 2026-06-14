#pragma once

#include <string>
#include <vector>

namespace ECS
{
	struct Box
	{
		int hitsToBreak = 1;
		bool dropFruitPerHit = false;
		std::vector<std::string> fruits;
		std::string debrisTexture; // texture id of this box's Break sheet

		int hitsTaken = 0;       // runtime
		bool isBreaking = false; // runtime: playing the final Hit, destroyed when it ends

		float ejectSpeedX = 0.0f;  // horizontal launch speed of dropped fruit
		float ejectSpeedUp = 0.0f; // upward launch speed of dropped fruit

		std::string hitSound;
		std::string breakSound;
	};
}