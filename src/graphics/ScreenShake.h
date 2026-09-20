#pragma once

#include <SFML/System/Vector2.hpp>

// A small, self-contained camera-shake, decoupled from the gameplay Camera
// (which also clamps to level bounds -- menus/splash screens have none of
// that, they just want a wobble). Same trauma-squared falloff shape.
class ScreenShake
{
public:
	// Adds trauma (clamped to 1 total); repeated calls in quick succession
	// stack up into a stronger, still-bounded shake.
	void Add(float trauma);

	// Decays trauma and rolls a fresh offset; call once per frame.
	void Update(float deltaTime);

	sf::Vector2f GetOffset() const { return offset; }

private:
	static constexpr float MaxOffset = 5.0f; // pixels at full trauma
	static constexpr float DecayPerSecond = 2.2f;

	float trauma = 0.0f;
	sf::Vector2f offset;
};
