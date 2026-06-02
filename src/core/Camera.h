#pragma once

#include <SFML/System/Vector2.hpp>

class Camera
{
public:
	void SnapTo(sf::Vector2f target);     // set instantly, no interpolation (level start, teleports)
	void MoveTo(sf::Vector2f target);     // new center for this step; call once per Update
	void SetWorldSize(sf::Vector2f size); // level size in pixels, for edge clamping (zero = no clamp)

	sf::Vector2f GetRenderCenter(float interpolationFactor) const;

private:
	sf::Vector2f Clamp(sf::Vector2f target) const;

	sf::Vector2f previousCenter;
	sf::Vector2f currentCenter;
	sf::Vector2f worldSize = { 0.0f, 0.0f };
};