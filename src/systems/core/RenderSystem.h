#pragma once

struct Resources;
class VirtualScreen;

namespace ECS
{
	class Registry;

	class RenderSystem
	{
	public:
		RenderSystem(Registry& registry, Resources& resources, VirtualScreen& virtualScreen);

		void Render(float interpolationFactor);

	private:
		Registry& registry;
		Resources& resources;
		VirtualScreen& virtualScreen;
	};
}