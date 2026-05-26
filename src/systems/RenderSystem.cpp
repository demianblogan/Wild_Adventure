#include "RenderSystem.h"

#include "components/PreviousTransform.h"
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
		[this, interpolationFactor](Entity entity, Transform& transform, Sprite& sprite)
		{
			float renderX = transform.x;
			float renderY = transform.y;

			if (registry.Has<PreviousTransform>(entity))
			{
				const PreviousTransform& previous = registry.Get<PreviousTransform>(entity);
				renderX = previous.x + (transform.x - previous.x) * interpolationFactor;
				renderY = previous.y + (transform.y - previous.y) * interpolationFactor;
			}

			sf::Sprite drawable(resources.textures.Get(sprite.textureName));
			drawable.setPosition({ renderX, renderY });
			window.draw(drawable);
		});
}