#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
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

	void SetCameraCenter(float x, float y);

	void UpdateMousePosition(sf::Vector2i windowPosition, sf::RenderWindow& window);
	sf::Vector2f GetMousePosition() const { return mousePosition; }

	static constexpr unsigned int WIDTH = 640;
	static constexpr unsigned int HEIGHT = 360;

private:
	sf::Vector2f MapWindowToVirtual(sf::Vector2i windowPosition, sf::RenderWindow& window) const;

	sf::RenderTexture renderTexture;
	sf::View camera;
	sf::Vector2f mousePosition;
};