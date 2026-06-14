#pragma once

#include "graphics/ColorGrading.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>

namespace sf
{
	class RenderTarget;
	class RenderWindow;
}

class VirtualScreen
{
public:
	VirtualScreen();

	void Clear();
	sf::RenderTarget& GetRenderTarget();
	void Display();
	void RenderToWindow(sf::RenderWindow& window);

	// Applies to every subsequent frame until changed; pass {} to turn the
	// effect off. Silently ignored when shaders are unsupported.
	void SetColorGrading(const ColorGrading& grading);

	// Gaussian-blurs everything drawn so far; whatever is drawn afterwards
	// (menu UI) stays sharp. More iterations widen the blur. Silently does
	// nothing when shaders are unsupported.
	void BlurContents(int iterations = 3);

	// Bloom: glowing things are drawn into this extra layer as well; its view
	// follows SetCameraCenter, so world and screen space both line up.
	sf::RenderTarget& GetGlowTarget();

	// Render states for drawing into the glow layer as a flat silhouette of
	// the given color — for auras whose color should not depend on the
	// sprite's own pixels. Falls back to plain states without shader support.
	sf::RenderStates GlowSilhouetteStates(sf::Color color);

	// Turns the glow layer into a pulsing aura around the drawn silhouettes
	// (the silhouette itself is cut out, so the object stays untouched) and
	// adds it onto the scene, then clears the layer. Call once everything
	// that should bloom this frame has been drawn (and before any later
	// pass — e.g. the pause blur — that must include it). The strength
	// scales the aura; UI buttons use GLOW_UI_STRENGTH since their darker
	// edge pixels spill a much fainter halo than the bright fruit sprites.
	void CompositeGlow(float strength = 1.0f);

	static constexpr float GLOW_UI_STRENGTH = 1.8f;

	void SetCameraCenter(float x, float y);

	void UpdateMousePosition(sf::Vector2i windowPosition, sf::RenderWindow& window);
	sf::Vector2f GetMousePosition() const { return mousePosition; }

	static constexpr unsigned int WIDTH = 480;
	static constexpr unsigned int HEIGHT = 270;

private:
	sf::Vector2f MapWindowToVirtual(sf::Vector2i windowPosition, sf::RenderWindow& window) const;

	sf::RenderTexture renderTexture;
	sf::View camera;
	sf::Vector2f mousePosition;

	sf::Shader gradingShader;
	bool gradingSupported = false; // shader compiled and usable on this machine
	bool gradingActive = false;    // current parameters differ from identity
	bool heatActive = false;       // heat haze on: the time uniform ticks every frame
	bool waterActive = false;      // underwater effects on: the time uniform ticks every frame
	sf::Clock effectClock;         // drives the heat haze animation

	sf::Shader blurShader;
	bool blurSupported = false;
	sf::RenderTexture blurTexture; // ping-pong partner for the blur passes

	sf::RenderTexture glowTexture; // bloom sources accumulate here
	sf::RenderTexture haloTexture; // blurred glow with the silhouettes cut out
	bool glowUsed = false;         // something was drawn into the glow layer

	sf::Shader silhouetteShader;   // flat-color silhouette for tinted auras
	bool silhouetteSupported = false;

	sf::Shader compositeShader;    // hue-preserving additive blend of the aura
	bool compositeSupported = false;

	static constexpr int   GLOW_BLUR_ITERATIONS = 3;
	static constexpr float GLOW_BOOST = 3.0f;       // spill coverage multiplier (capped at full color)
	static constexpr float GLOW_INTENSITY = 1.0f;   // overall aura brightness
	static constexpr float GLOW_PULSE_SPEED = 4.0f; // radians per second
	static constexpr float GLOW_PULSE_MIN = 0.45f;  // aura strength at the dim end of the pulse
};