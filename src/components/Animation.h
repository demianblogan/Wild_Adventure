#pragma once

struct Animation
{
	int frameCount = 1;
	float frameDuration = 0.1f;
	bool isLooping = true;

	int currentFrame = 0;
	float elapsedTime = 0.0f;
};