#include "DamageSystem.h"

#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/Hazard.h"
#include "components/Health.h"
#include "components/Hitbox.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"

#include <cmath>
#include <vector>

namespace ECS
{
	DamageSystem::DamageSystem(Registry& registry)
		: registry(registry)
	{}

	void DamageSystem::Update(float deltaTime)
	{
		struct HazardBox { float left; float right; float top; float bottom; int damage; };

		std::vector<HazardBox> hazards;
		registry.ForEach<Hazard, Hitbox, Transform>(
			[&hazards](Entity, Hazard& hazard, Hitbox& hitbox, Transform& transform)
			{
				const float halfWidth = hitbox.width / 2.0f;
				hazards.push_back({ transform.x - halfWidth, transform.x + halfWidth,
					transform.y - hitbox.height, transform.y, hazard.damage });
			});

		registry.ForEach<Player, Transform, Collider, Health, Velocity>(
			[this, deltaTime, &hazards](Entity entity, Player&, Transform& transform,
				Collider& collider, Health& health, Velocity& velocity)
			{
				if (health.invulnerabilityTimer > 0.0f)
					health.invulnerabilityTimer -= deltaTime;
				if (health.hitStunTimer > 0.0f)
					health.hitStunTimer -= deltaTime;

				if (health.invulnerabilityTimer > 0.0f)
					return;

				const float halfWidth = collider.width / 2.0f;
				const float left = transform.x - halfWidth;
				const float right = transform.x + halfWidth;
				const float top = transform.y - collider.height;
				const float bottom = transform.y;

				for (const HazardBox& hazard : hazards)
				{
					const bool overlap = left < hazard.right && right > hazard.left
						&& top < hazard.bottom && bottom > hazard.top;

					if (!overlap)
						continue;

					health.current -= hazard.damage;

					const float hazardCenterX = (hazard.left + hazard.right) / 2.0f;
					const float hazardCenterY = (hazard.top + hazard.bottom) / 2.0f;

					const float dx = transform.x - hazardCenterX;
					const float dy = (transform.y - collider.height / 2.0f) - hazardCenterY;

					if (std::abs(dx) > std::abs(dy)) // hazard to the side -> up and away
					{
						const float sign = (dx > 0.0f) ? 1.0f : -1.0f;
						velocity.x = sign * health.knockbackSpeed;
						velocity.y = -health.knockbackSpeed;
					}
					else if (dy < 0.0f) // hazard below -> straight up
					{
						velocity.x = 0.0f;
						velocity.y = -health.knockbackSpeed;
					}
					else // hazard above -> straight down
					{
						velocity.x = 0.0f;
						velocity.y = health.knockbackSpeed;
					}

					health.invulnerabilityTimer = health.invulnerabilityDuration;
					health.hitStunTimer = health.hitStunDuration;

					if (registry.Has<AnimationState>(entity))
						registry.Get<AnimationState>(entity).current = "Hit";

					break;
				}
			});
	}
}