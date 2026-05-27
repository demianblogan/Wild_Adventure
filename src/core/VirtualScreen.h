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

	static constexpr unsigned int WIDTH = 640;
	static constexpr unsigned int HEIGHT = 360;

private:
	sf::RenderTexture renderTexture;
	sf::View camera;
};