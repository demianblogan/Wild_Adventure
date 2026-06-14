#include "VirtualScreen.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

VirtualScreen::VirtualScreen()
	: renderTexture({ WIDTH, HEIGHT })
	, blurTexture({ WIDTH, HEIGHT })
	, glowTexture({ WIDTH, HEIGHT })
	, haloTexture({ WIDTH, HEIGHT })
	, camera(sf::FloatRect({ 0.0f, 0.0f }, { static_cast<float>(WIDTH), static_cast<float>(HEIGHT) }))
{
	renderTexture.setSmooth(false);
	renderTexture.setView(camera);
	blurTexture.setSmooth(false);
	glowTexture.setSmooth(false);
	haloTexture.setSmooth(false);
	glowTexture.setView(camera);
	glowTexture.clear(sf::Color::Transparent);

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

	silhouetteSupported = sf::Shader::isAvailable()
		&& silhouetteShader.loadFromFile("assets/shaders/glow_silhouette.frag", sf::Shader::Type::Fragment);

	if (silhouetteSupported)
		silhouetteShader.setUniform("texture", sf::Shader::CurrentTexture);

	compositeSupported = sf::Shader::isAvailable()
		&& compositeShader.loadFromFile("assets/shaders/glow_composite.frag", sf::Shader::Type::Fragment);

	if (compositeSupported)
		compositeShader.setUniform("texture", sf::Shader::CurrentTexture);
}

sf::RenderStates VirtualScreen::GlowSilhouetteStates(sf::Color color)
{
	sf::RenderStates states;

	if (!silhouetteSupported)
		return states; // aura falls back to the sprite's own colors

	silhouetteShader.setUniform("glowColor", sf::Glsl::Vec3(
		color.r / 255.0f, color.g / 255.0f, color.b / 255.0f));
	states.shader = &silhouetteShader;

	return states;
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
	waterActive = gradingActive && grading.water > 0.0f;

	if (!gradingActive)
		return;

	gradingShader.setUniform("tint", grading.tint);
	gradingShader.setUniform("brightness", grading.brightness);
	gradingShader.setUniform("saturation", grading.saturation);
	gradingShader.setUniform("contrast", grading.contrast);

	// The shader works in UV space; the data file speaks virtual pixels.
	gradingShader.setUniform("heatStrength", grading.heat / static_cast<float>(WIDTH));
	gradingShader.setUniform("water", grading.water);
	gradingShader.setUniform("time", 0.0f);
}

void VirtualScreen::Clear()
{
	renderTexture.clear(sf::Color::Black);

	if (glowUsed)
	{
		// Stale glow that was never composited last frame.
		glowTexture.clear(sf::Color::Transparent);
		glowUsed = false;
	}
}

sf::RenderTarget& VirtualScreen::GetGlowTarget()
{
	glowUsed = true;
	return glowTexture;
}

void VirtualScreen::CompositeGlow(float strength)
{
	if (!glowUsed)
		return;

	glowUsed = false;

	// Bloom needs the blur and composite shaders; without them drop the layer.
	if (!blurSupported || !compositeSupported)
	{
		glowTexture.clear(sf::Color::Transparent);
		return;
	}

	const sf::View previousMainView = renderTexture.getView();
	const sf::View screenView(sf::FloatRect(
		{ 0.0f, 0.0f }, { static_cast<float>(WIDTH), static_cast<float>(HEIGHT) }));

	sf::RenderStates blurStates;
	blurStates.shader = &blurShader;
	blurStates.blendMode = sf::BlendNone;

	glowTexture.display();

	// Widen the glow sources; the untouched original stays in glowTexture,
	// the blurred result lands in haloTexture.
	const sf::Texture* source = &glowTexture.getTexture();
	for (int i = 0; i < GLOW_BLUR_ITERATIONS; i++)
	{
		blurShader.setUniform("direction", sf::Glsl::Vec2(1.0f / WIDTH, 0.0f));
		blurTexture.draw(sf::Sprite(*source), blurStates);
		blurTexture.display();

		blurShader.setUniform("direction", sf::Glsl::Vec2(0.0f, 1.0f / HEIGHT));
		haloTexture.draw(sf::Sprite(blurTexture.getTexture()), blurStates);
		haloTexture.display();

		source = &haloTexture.getTexture();
	}

	// Cut the original silhouettes out of the blur: only the spill that crept
	// beyond the sprite remains, so the object itself stays crisp.
	const sf::BlendMode cutOut(sf::BlendMode::Factor::Zero, sf::BlendMode::Factor::OneMinusSrcAlpha);
	haloTexture.draw(sf::Sprite(glowTexture.getTexture()), cutOut);
	haloTexture.display();

	// Add the aura onto the scene, breathing between dim and full strength.
	// The composite shader boosts the spill *coverage* (capped at the pure
	// aura color) instead of stacking additive passes, so the inner aura
	// stays at the chosen hue instead of clamping every channel to white.
	const float seconds = effectClock.getElapsedTime().asSeconds();
	const float pulse = GLOW_PULSE_MIN
		+ (1.0f - GLOW_PULSE_MIN) * 0.5f * (1.0f + std::sin(seconds * GLOW_PULSE_SPEED));

	compositeShader.setUniform("boost", GLOW_BOOST * strength);
	compositeShader.setUniform("intensity", GLOW_INTENSITY * pulse);

	sf::RenderStates additive;
	additive.blendMode = sf::BlendMode(sf::BlendMode::Factor::One, sf::BlendMode::Factor::One);
	additive.shader = &compositeShader;

	renderTexture.setView(screenView);
	renderTexture.draw(sf::Sprite(haloTexture.getTexture()), additive);
	renderTexture.setView(previousMainView);

	// Leave the layer empty for the next user (possibly later this frame).
	glowTexture.clear(sf::Color::Transparent);
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
	glowTexture.setView(camera); // glow sources are drawn in the same space
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

	if (heatActive || waterActive)
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