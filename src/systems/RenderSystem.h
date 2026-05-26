#pragma once

namespace sf
{
	class RenderWindow;
}

class Registry;
struct Resources;

class RenderSystem
{
public:
	RenderSystem(Registry& registry, Resources& resources, sf::RenderWindow& window);

	void Render(float interpolationFactor);

private:
	Registry& registry;
	Resources& resources;
	sf::RenderWindow& window;
};