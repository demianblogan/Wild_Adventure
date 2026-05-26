#include "RenderSystem.h"

#include "Context.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "core/Resources.h"
#include "core/ecs/Registry.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>

RenderSystem::RenderSystem(Registry& registry, Resources& resources, sf::RenderWindow& window)
	: registry(registry)
	, resources(resources)
	, window(window)
{}

void RenderSystem::Render(float interpolationFactor)
{
	registry.ForEach<Transform, Sprite>(
		[this](Entity entity, Transform& transform, Sprite& sprite)
		{
			sf::Sprite drawable(resources.textures.Get(sprite.textureName));
			drawable.setPosition({ transform.x, transform.y });
			window.draw(drawable);
		}
	);
}