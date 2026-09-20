#include "graphics/ScreenShake.h"

#include "core/Random.h"

#include <algorithm>

void ScreenShake::Add(float trauma)
{
	this->trauma = std::min(1.0f, this->trauma + trauma);
}

void ScreenShake::Update(float deltaTime)
{
	if (trauma <= 0.0f)
	{
		offset = { 0.0f, 0.0f };
		return;
	}

	trauma = std::max(0.0f, trauma - DecayPerSecond * deltaTime);

	const float magnitude = MaxOffset * trauma * trauma;
	offset = { Random::Float(-magnitude, magnitude), Random::Float(-magnitude, magnitude) };
}
