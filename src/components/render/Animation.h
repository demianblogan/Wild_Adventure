#pragma once

#include "AnimationData.h"

#include <string>

namespace ECS
{
	struct Animation
	{
		AnimationData data;
		std::string playingState;
		int currentFrame = 0;
		float elapsedTime = 0.0f;
		bool isFinished = false; // set when a non-looping animation reaches its last frame
	};
}
