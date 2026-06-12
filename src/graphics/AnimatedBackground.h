#pragma once

#include <SFML/Graphics/Color.hpp>
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
	void SetTint(sf::Color color); // multiplies the texture; darkens cave levels

	void Update(float deltaTime);

	// worldAnchor pins the pattern to world coordinates (pass the camera
	// center): camera movement then never changes the background's apparent
	// scroll speed. With the default zero anchor it is fixed to the screen.
	void Draw(sf::RenderTarget& target, sf::Vector2f worldAnchor = { 0.0f, 0.0f });

private:
	Resources& resources;
	std::string textureName;
	Direction direction = Direction::Down;
	float speed = 20.0f;
	sf::Color tint = sf::Color::White;
	sf::Vector2f scrollOffset = { 0.0f, 0.0f };
};