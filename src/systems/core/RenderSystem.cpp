#include "RenderSystem.h"

#include "components/render/Animation.h"
#include "components/physics/Facing.h"
#include "components/combat/Health.h"
#include "components/physics/PreviousTransform.h"
#include "components/render/Sprite.h"
#include "components/physics/Transform.h"
#include "components/physics/Rotation.h"
#include "components/tags/Frozen.h"
#include "components/render/Hidden.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "core/ecs/Registry.h"

#include <SFML/System/Angle.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <cmath>
#include <iostream> // DEBUG

namespace ECS
{
	RenderSystem::RenderSystem(Registry& registry, Resources& resources, VirtualScreen& virtualScreen)
		: registry(registry)
		, resources(resources)
		, virtualScreen(virtualScreen)
	{}

	void RenderSystem::Render(float interpolationFactor)
	{
		sf::RenderTarget& renderTarget = virtualScreen.GetRenderTarget();

		registry.ForEach<Transform, Sprite>(
			[this, interpolationFactor, &renderTarget](Entity entity, Transform& transform, Sprite& sprite)
			{
				if (registry.Has<Frozen>(entity))
					return;

				// An invisible ghost (and anything else flagged Hidden) is not drawn.
				if (registry.Has<Hidden>(entity))
					return;

				// Blink while invulnerable: hide on alternate short windows.
				if (registry.Has<Health>(entity))
				{
					const Health& health = registry.Get<Health>(entity);
					if (health.invulnerabilityTimer > 0.0f && health.current > 0 && std::fmod(health.invulnerabilityTimer, 0.16f) < 0.08f)
						return;
				}

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

				sf::Vector2f scale = { sprite.scaleX, sprite.scaleY };

				if (registry.Has<Facing>(entity))
				{
					const Facing& facing = registry.Get<Facing>(entity);
					if (facing.isLookingRight != facing.isTextureRight)
						scale.x = -scale.x;
				}

				drawable.setScale(scale);

				if (registry.Has<Rotation>(entity))
					drawable.setRotation(sf::degrees(registry.Get<Rotation>(entity).angle));

				drawable.setPosition({ std::floor(renderX + sprite.offsetX), std::floor(renderY + sprite.offsetY) });

				renderTarget.draw(drawable);

				if (sprite.glow)
					virtualScreen.GetGlowTarget().draw(drawable,
						virtualScreen.GlowSilhouetteStates(sprite.glowColor));
			});
	}
}