#pragma once

namespace sf
{
	class RenderTarget;
}

class Registry;
struct Resources;

class RenderSystem
{
public:
	RenderSystem(Registry& registry, Resources& resources, sf::RenderTarget& renderTarget);

	void Render(float interpolationFactor);

private:
	Registry& registry;
	Resources& resources;
	sf::RenderTarget& renderTarget;
};