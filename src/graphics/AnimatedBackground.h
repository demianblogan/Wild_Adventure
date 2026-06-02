#pragma once

#include <SFML/System/Vector2.hpp>

#include <string>

namespace sf
{
	class RenderTarget;
}

struct Resources;

class AnimatedBackground
{
public:
	enum class Direction
	{
		Up,
		Down,
		Left,
		Right
	};

	AnimatedBackground(Resources& resources);

	void SetTexture(const std::string& textureName);
	void SetDirection(Direction direction);
	void SetSpeed(float pixelsPerSecond);

	void Update(float deltaTime);
	void Draw(sf::RenderTarget& target);

private:
	Resources& resources;
	std::string textureName;
	Direction direction = Direction::Down;
	float speed = 20.0f;
	sf::Vector2f scrollOffset = { 0.0f, 0.0f };
};