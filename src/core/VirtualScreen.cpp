#include "VirtualScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

VirtualScreen::VirtualScreen()
	: renderTexture({ WIDTH, HEIGHT })
	, camera(sf::FloatRect({ 0.0f, 0.0f }, { static_cast<float>(WIDTH), static_cast<float>(HEIGHT) }))
{
	renderTexture.setSmooth(false);
	renderTexture.setView(camera);
}

void VirtualScreen::Clear()
{
	renderTexture.clear(sf::Color::Black);
}

sf::RenderTarget& VirtualScreen::GetRenderTarget()
{
	return renderTexture;
}

void VirtualScreen::Display()
{
	renderTexture.display();
}

void VirtualScreen::SetCameraCenter(float x, float y)
{
	camera.setCenter({ x, y });
	renderTexture.setView(camera);
}

void VirtualScreen::UpdateMousePosition(sf::Vector2i windowPosition, sf::RenderWindow& window)
{
	mousePosition = MapWindowToVirtual(windowPosition, window);
}

void VirtualScreen::RenderToWindow(sf::RenderWindow& window)
{
	const sf::Vector2u windowSize = window.getSize();

	const unsigned int scaleX = windowSize.x / WIDTH;
	const unsigned int scaleY = windowSize.y / HEIGHT;
	const unsigned int scale = std::min(scaleX, scaleY);

	const float scaledWidth = static_cast<float>(WIDTH * scale);
	const float scaledHeight = static_cast<float>(HEIGHT * scale);

	sf::Sprite screenSprite(renderTexture.getTexture());
	screenSprite.setScale({ static_cast<float>(scale), static_cast<float>(scale) });
	screenSprite.setPosition({
		(static_cast<float>(windowSize.x) - scaledWidth) / 2.0f,
		(static_cast<float>(windowSize.y) - scaledHeight) / 2.0f });

	window.draw(screenSprite);
}

sf::Vector2f VirtualScreen::MapWindowToVirtual(sf::Vector2i windowPosition, sf::RenderWindow& window) const
{
	const sf::Vector2u windowSize = window.getSize();

	const unsigned int scale = std::min(windowSize.x / WIDTH, windowSize.y / HEIGHT);

	const float scaledWidth = static_cast<float>(WIDTH * scale);
	const float scaledHeight = static_cast<float>(HEIGHT * scale);

	const float offsetX = (static_cast<float>(windowSize.x) - scaledWidth) / 2.0f;
	const float offsetY = (static_cast<float>(windowSize.y) - scaledHeight) / 2.0f;

	return {
		(static_cast<float>(windowPosition.x) - offsetX) / static_cast<float>(scale),
		(static_cast<float>(windowPosition.y) - offsetY) / static_cast<float>(scale) };
}