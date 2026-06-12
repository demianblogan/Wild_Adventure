#pragma once

#include "graphics/ColorGrading.h"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/View.hpp>

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
};