#pragma once

namespace sf
{
	class RenderTarget;
}

struct Resources;

namespace ECS
{
	class Registry;

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
}