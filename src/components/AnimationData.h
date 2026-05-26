#pragma once

#include <string>

struct AnimationData
{
	std::string textureName;
	int frameCount = 1;
	float frameDuration = 0.1f;
	bool isLooping = true;
};