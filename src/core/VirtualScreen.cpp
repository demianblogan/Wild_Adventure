#include "VirtualScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <algorithm>
#include <cmath>

VirtualScreen::VirtualScreen()
	: renderTexture({ WIDTH, HEIGHT })
	, blurTexture({ WIDTH, HEIGHT })
	, camera(sf::FloatRect({ 0.0f, 0.0f }, { static_cast<float>(WIDTH), static_cast<float>(HEIGHT) }))
{
	renderTexture.setSmooth(false);
	renderTexture.setView(camera);
	blurTexture.setSmooth(false);

	// Post effects degrade gracefully: without shader support the virtual
	// screen is simply blitted as before.
	gradingSupported = sf::Shader::isAvailable()
		&& gradingShader.loadFromFile("assets/shaders/color_grading.frag", sf::Shader::Type::Fragment);

	if (gradingSupported)
		gradingShader.setUniform("texture", sf::Shader::CurrentTexture);

	blurSupported = sf::Shader::isAvailable()
		&& blurShader.loadFromFile("assets/shaders/blur.frag", sf::Shader::Type::Fragment);

	if (blurSupported)
		blurShader.setUniform("texture", sf::Shader::CurrentTexture);
}

void VirtualScreen::BlurContents(int iterations)
{
	if (!blurSupported)
		return;

	// The blur passes copy full screens; views must map 1:1, independent of
	// whatever camera a state has set.
	const sf::View previousView = renderTexture.getView();
	renderTexture.setView(sf::View(sf::FloatRect(
		{ 0.0f, 0.0f }, { static_cast<float>(WIDTH), static_cast<float>(HEIGHT) })));

	sf::RenderStates states;
	states.shader = &blurShader;
	states.blendMode = sf::BlendNone;

	for (int i = 0; i < iterations; i++)
	{
		renderTexture.display(); // make the content drawn so far samplable

		blurShader.setUniform("direction", sf::Glsl::Vec2(1.0f / WIDTH, 0.0f));
		blurTexture.draw(sf::Sprite(renderTexture.getTexture()), states);
		blurTexture.display();

		blurShader.setUniform("direction", sf::Glsl::Vec2(0.0f, 1.0f / HEIGHT));
		renderTexture.draw(sf::Sprite(blurTexture.getTexture()), states);
	}

	renderTexture.setView(previousView);
}

void VirtualScreen::SetColorGrading(const ColorGrading& grading)
{
	gradingActive = gradingSupported && !grading.IsIdentity();
	heatActive = gradingActive && grading.heat > 0.0f;

	if (!gradingActive)
		return;

	gradingShader.setUniform("tint", grading.tint);
	gradingShader.setUniform("brightness", grading.brightness);
	gradingShader.setUniform("saturation", grading.saturation);
	gradingShader.setUniform("contrast", grading.contrast);

	// The shader works in UV space; the data file speaks virtual pixels.
	gradingShader.setUniform("heatStrength", grading.heat / static_cast<float>(WIDTH));
	gradingShader.setUniform("time", 0.0f);
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
	const sf::Vector2f windowSize(window.getSize());

	// Draw with a 1:1 pixel view so positions map directly to window pixels,
	// regardless of any earlier view change, window resize, or recreation.
	window.setView(sf::View(sf::FloatRect({ 0.0f, 0.0f }, windowSize)));

	// Fractional scale: fill the window as much as possible while keeping the
	// 16:9 aspect ratio. The leftover space becomes letterbox bars.
	const float scale = std::min(windowSize.x / WIDTH, windowSize.y / HEIGHT);

	const float scaledWidth = WIDTH * scale;
	const float scaledHeight = HEIGHT * scale;

	sf::Sprite screenSprite(renderTexture.getTexture());
	screenSprite.setScale({ scale, scale });
	screenSprite.setPosition({
		(windowSize.x - scaledWidth) / 2.0f,
		(windowSize.y - scaledHeight) / 2.0f });

	if (heatActive)
	{
		// Wrap at 200*pi so the float stays precise over long sessions; the
		// shader's wave speeds are multiples of 0.1, which keeps the wrap
		// seamless (200*pi * 0.1 is a whole number of 2*pi periods).
		const float time = std::fmod(effectClock.getElapsedTime().asSeconds(), 200.0f * 3.14159265f);
		gradingShader.setUniform("time", time);
	}

	if (gradingActive)
		window.draw(screenSprite, &gradingShader);
	else
		window.draw(screenSprite);
}

sf::Vector2f VirtualScreen::MapWindowToVirtual(sf::Vector2i windowPosition, sf::RenderWindow& window) const
{
	const sf::Vector2f windowSize(window.getSize());

	// Must match RenderToWindow exactly, so clicks land on the drawn elements.
	const float scale = std::min(windowSize.x / WIDTH, windowSize.y / HEIGHT);

	const float scaledWidth = WIDTH * scale;
	const float scaledHeight = HEIGHT * scale;

	const float offsetX = (windowSize.x - scaledWidth) / 2.0f;
	const float offsetY = (windowSize.y - scaledHeight) / 2.0f;

	return {
		(static_cast<float>(windowPosition.x) - offsetX) / scale,
		(static_cast<float>(windowPosition.y) - offsetY) / scale };
}