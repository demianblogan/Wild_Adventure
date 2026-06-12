#pragma once

#include <SFML/System/Vector2.hpp>

#include <random>

class Camera
{
public:
	void SnapTo(sf::Vector2f target);     // set instantly, no interpolation (level start, teleports)
	void MoveTo(sf::Vector2f target);     // new center for this step; call once per Update
	void SetWorldSize(sf::Vector2f size); // level size in pixels, for edge clamping (zero = no clamp)

	// Adds shake trauma (clamped to 1). The offset scales with trauma squared,
	// so light bumps stay subtle while stacked hits rattle the screen.
	void Shake(float trauma);

	// Decays trauma and rolls a fresh shake offset; call once per fixed step.
	void Update(float deltaTime);

	sf::Vector2f GetRenderCenter(float interpolationFactor) const;

private:
	sf::Vector2f Clamp(sf::Vector2f target) const;

	sf::Vector2f previousCenter;
	sf::Vector2f currentCenter;
	sf::Vector2f worldSize = { 0.0f, 0.0f };

	float trauma = 0.0f;                       // 0..1
	sf::Vector2f shakeOffset = { 0.0f, 0.0f }; // current frame's offset in pixels
	std::minstd_rand randomEngine{ std::random_device{}() };

	static constexpr float MAX_SHAKE_OFFSET = 6.0f; // pixels at full trauma
	static constexpr float TRAUMA_DECAY = 2.0f;     // trauma lost per second
};
