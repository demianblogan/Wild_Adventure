#include "Camera.h"

#include "core/VirtualScreen.h"

#include <algorithm>
#include <cmath>

void Camera::SnapTo(sf::Vector2f target)
{
	currentCenter = Clamp(target);
	previousCenter = currentCenter;
}

void Camera::MoveTo(sf::Vector2f target)
{
	previousCenter = currentCenter;
	currentCenter = Clamp(target);
}

void Camera::SetWorldSize(sf::Vector2f size)
{
	worldSize = size;
}

void Camera::Shake(float trauma)
{
	this->trauma = std::min(1.0f, this->trauma + trauma);
}

void Camera::Update(float deltaTime)
{
	if (trauma <= 0.0f)
		return;

	trauma = std::max(0.0f, trauma - TRAUMA_DECAY * deltaTime);

	const float magnitude = MAX_SHAKE_OFFSET * trauma * trauma;
	std::uniform_real_distribution<float> offset(-magnitude, magnitude);

	shakeOffset = { offset(randomEngine), offset(randomEngine) };
}

sf::Vector2f Camera::GetRenderCenter(float interpolationFactor) const
{
	const sf::Vector2f interpolated =
		previousCenter + (currentCenter - previousCenter) * interpolationFactor;

	// Re-clamp so the shake never exposes the area beyond the level edges.
	const sf::Vector2f shaken = Clamp(interpolated + shakeOffset);

	// Floor to whole virtual pixels so tiles and sprites stay crisp.
	return { std::floor(shaken.x), std::floor(shaken.y) };
}

sf::Vector2f Camera::Clamp(sf::Vector2f target) const
{
	if (worldSize.x <= 0.0f || worldSize.y <= 0.0f)
		return target;

	const float halfWidth = VirtualScreen::WIDTH / 2.0f;
	const float halfHeight = VirtualScreen::HEIGHT / 2.0f;

	sf::Vector2f result = target;

	if (worldSize.x <= VirtualScreen::WIDTH)
		result.x = worldSize.x / 2.0f; // level narrower than the view: center it
	else
		result.x = std::clamp(target.x, halfWidth, worldSize.x - halfWidth);

	if (worldSize.y <= VirtualScreen::HEIGHT)
		result.y = worldSize.y / 2.0f;
	else
		result.y = std::clamp(target.y, halfHeight, worldSize.y - halfHeight);

	return result;
}