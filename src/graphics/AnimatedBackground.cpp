#include "AnimatedBackground.h"

#include "core/Resources.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <cmath>

AnimatedBackground::AnimatedBackground(Resources& resources)
	: resources(resources)
{}

void AnimatedBackground::SetTexture(const std::string& name)
{
	textureName = name;

	sf::Texture& texture = resources.textures.Get(textureName);
	texture.setRepeated(true);
}

void AnimatedBackground::SetDirection(Direction newDirection)
{
	direction = newDirection;
}

void AnimatedBackground::SetSpeed(float pixelsPerSecond)
{
	speed = pixelsPerSecond;
}

void AnimatedBackground::SetTint(sf::Color color)
{
	tint = color;
}

void AnimatedBackground::Update(float deltaTime)
{
	const float delta = speed * deltaTime;

	switch (direction)
	{
	case Direction::Down:
		scrollOffset.y += delta;
		break;
	case Direction::Up:
		scrollOffset.y -= delta;
		break;
	case Direction::Right:
		scrollOffset.x += delta;
		break;
	case Direction::Left:
		scrollOffset.x -= delta;
		break;
	}
}

void AnimatedBackground::Draw(sf::RenderTarget& target, sf::Vector2f worldAnchor)
{
	if (textureName.empty())
		return;

	const sf::Texture& texture = resources.textures.Get(textureName);
	const sf::Vector2u textureSize = texture.getSize();
	const sf::Vector2u screenSize = target.getSize();

	// Subtracting the anchor shifts the pattern opposite to the camera, so
	// the background stays put in the world while only its own scroll moves.
	const int offsetX = static_cast<int>(std::fmod(scrollOffset.x - worldAnchor.x, static_cast<float>(textureSize.x)));
	const int offsetY = static_cast<int>(std::fmod(scrollOffset.y - worldAnchor.y, static_cast<float>(textureSize.y)));

	sf::Sprite sprite(texture);
	sprite.setTextureRect(sf::IntRect(
		{ -offsetX, -offsetY },
		{ static_cast<int>(screenSize.x), static_cast<int>(screenSize.y) }));
	sprite.setColor(tint);

	target.draw(sprite);
}