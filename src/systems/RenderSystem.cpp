#include "RenderSystem.h"

#include "components/Animation.h"
#include "components/Facing.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "core/Resources.h"
#include "core/ecs/Registry.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>

namespace ECS
{
	RenderSystem::RenderSystem(Registry& registry, Resources& resources, sf::RenderTarget& renderTarget)
		: registry(registry)
		, resources(resources)
		, renderTarget(renderTarget)
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

				const sf::Texture& texture = resources.textures.Get(sprite.textureName);
				sf::Sprite drawable(texture);

				int frameWidth = static_cast<int>(texture.getSize().x);
				const int frameHeight = static_cast<int>(texture.getSize().y);

				if (registry.Has<Animation>(entity))
				{
					const Animation& animation = registry.Get<Animation>(entity);
					frameWidth = static_cast<int>(texture.getSize().x) / animation.data.frameCount;

					const int displayedFrame = animation.data.isReversed
						? (animation.data.frameCount - 1 - animation.currentFrame)
						: animation.currentFrame;

					const sf::IntRect frameRect(
						{ displayedFrame * frameWidth, 0 },
						{ frameWidth, frameHeight });

					drawable.setTextureRect(frameRect);
				}

				drawable.setOrigin({ frameWidth / 2.0f, static_cast<float>(frameHeight) });

				if (registry.Has<Facing>(entity))
				{
					const Facing& facing = registry.Get<Facing>(entity);
					if (facing.isLookingRight != facing.isTextureRight)
						drawable.setScale({ -1.0f, 1.0f });
				}

				// Snap to whole virtual pixels so moving sprites stay crisp on upscale.
				drawable.setPosition({ std::floor(renderX), std::floor(renderY) });

				renderTarget.draw(drawable);
			});
	}
}