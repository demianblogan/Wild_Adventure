#include "RockHeadSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/physics/Collider.h"
#include "components/physics/CollisionState.h"
#include "components/combat/Health.h"
#include "components/physics/PreviousTransform.h"
#include "components/traps/RockHead.h"
#include "components/physics/Solid.h"
#include "components/physics/Transform.h"
#include "components/physics/Velocity.h"
#include "core/GamepadHaptics.h"
#include "core/HapticCues.h"
#include "core/ecs/Registry.h"
#include "systems/core/PlayerQuery.h"
#include "tilemap/Tilemap.h"

#include <algorithm>
#include <cmath>
#include <string>

namespace ECS
{
	namespace
	{
		bool IsHitAnim(const std::string& state)
		{
			return state == "BottomHit" || state == "TopHit"
				|| state == "LeftHit" || state == "RightHit";
		}

		constexpr float RideSnapTolerance = 3.0f; // how close the rider's feet must be to the rock's top
		constexpr float EdgeEpsilon = 0.001f;      // nudges a tile-boundary sample inward, see PhysicsSystem
		constexpr float CrushInset = 2.0f;         // shrinks the crush check so standing on top isn't a crush
	}

	RockHeadSystem::RockHeadSystem(Registry& registry, const Tilemap& tilemap, Haptics::GamepadHaptics& gamepadHaptics)
		: registry(registry)
		, tilemap(tilemap)
		, gamepadHaptics(gamepadHaptics)
	{}

	void RockHeadSystem::Update(float deltaTime)
	{
		const Entity playerEntity = FindPlayer(registry);

		Transform*      playerTransform = nullptr;
		Collider*       playerCollider  = nullptr;
		Health*         playerHealth    = nullptr;
		Velocity*       playerVelocity  = nullptr;
		AnimationState* playerAnimState = nullptr;
		if (playerEntity != InvalidEntity)
		{
			playerTransform = &registry.Get<Transform>(playerEntity);
			playerCollider  = &registry.Get<Collider>(playerEntity);
			playerHealth    = &registry.Get<Health>(playerEntity);
			playerVelocity  = &registry.Get<Velocity>(playerEntity);
			if (registry.Has<AnimationState>(playerEntity))
				playerAnimState = &registry.Get<AnimationState>(playerEntity);
		}

		registry.ForEach<RockHead, Transform, PreviousTransform, Velocity, CollisionState, AnimationState, Solid>(
			[&](Entity entity, RockHead& rock, Transform& transform, PreviousTransform& previous,
				Velocity& velocity, CollisionState& collisionState, AnimationState& animState, Solid& solid)
			{
				// How far PhysicsSystem actually moved the rock this frame.
				const float deltaX = transform.x - previous.x;

				if (playerTransform != nullptr)
				{
					const float halfWidth  = solid.width / 2.0f;
					const float rockLeft   = transform.x - halfWidth;
					const float rockRight  = transform.x + halfWidth;
					const float rockTop    = transform.y - solid.height;
					const float pHalfWidth = playerCollider->width / 2.0f;

					const bool horizontallyOver =
						(playerTransform->x + pHalfWidth) > rockLeft &&
						(playerTransform->x - pHalfWidth) < rockRight;

					// Riding on top of a horizontal rock: carry the player along. (Vertical
					// carrying happens for free as the player snaps to the Solid's top.)
					if (rock.axis == RockHead::Axis::Horizontal && horizontallyOver
						&& std::abs(playerTransform->y - rockTop) < RideSnapTolerance && deltaX != 0.0f)
					{
						playerTransform->x += deltaX;

						// PhysicsSystem only pushes the player out of walls when they are
						// moving, so a still rider would otherwise be carried straight through
						// terrain. Clamp the carried player against the wall ahead.
						const float tileSize = static_cast<float>(tilemap.tileSize);
						const float pHalf    = playerCollider->width / 2.0f;
						const int   firstRow = static_cast<int>(std::floor((playerTransform->y - playerCollider->height) / tileSize));
						const int   lastRow  = static_cast<int>(std::floor((playerTransform->y - EdgeEpsilon) / tileSize));

						if (deltaX > 0.0f)
						{
							const int column = static_cast<int>(std::floor((playerTransform->x + pHalf) / tileSize));
							for (int row = firstRow; row <= lastRow; ++row)
								if (tilemap.IsSolid(column, row)) { playerTransform->x = column * tileSize - pHalf; break; }
						}
						else
						{
							const int column = static_cast<int>(std::floor((playerTransform->x - pHalf) / tileSize));
							for (int row = firstRow; row <= lastRow; ++row)
								if (tilemap.IsSolid(column, row)) { playerTransform->x = (column + 1) * tileSize + pHalf; break; }
						}
					}

					// Crush: the player's body is wedged inside the rock (the inset excludes
					// merely standing on top), so it has them pinned against the terrain.
					const float inset = CrushInset;
					const bool crushed =
						(playerTransform->x - pHalfWidth) < (rockRight - inset) &&
						(playerTransform->x + pHalfWidth) > (rockLeft + inset) &&
						(playerTransform->y - playerCollider->height) < (transform.y - inset) &&
						playerTransform->y > (rockTop + inset);

					if (crushed && playerHealth->current > 0 && playerHealth->invulnerabilityTimer <= 0.0f)
					{
						playerHealth->current -= 1;

						const float sign = (playerTransform->x >= transform.x) ? 1.0f : -1.0f;
						playerVelocity->x = sign * playerHealth->knockbackSpeed;
						playerVelocity->y = -playerHealth->knockbackSpeed;
						playerHealth->invulnerabilityTimer = playerHealth->invulnerabilityDuration;
						playerHealth->hitStunTimer         = playerHealth->hitStunDuration;
						if (playerAnimState)
							playerAnimState->current = "Hit";
					}
				}

				switch (rock.state)
				{
				case RockHead::State::Moving:
				{
					// Did PhysicsSystem stop us against the terrain in our travel direction?
					bool        wasHit     = false;
					const char* hitAnim = "Idle";
					if (rock.axis == RockHead::Axis::Horizontal)
					{
						if (collisionState.isOnWall && collisionState.wallDirection == rock.direction)
						{
							wasHit     = true;
							hitAnim = (rock.direction > 0) ? "RightHit" : "LeftHit";
						}
					}
					else if (rock.direction > 0 && collisionState.isOnGround)
					{
						wasHit     = true;
						hitAnim = "BottomHit";
					}
					else if (rock.direction < 0 && collisionState.isOnCeiling)
					{
						wasHit     = true;
						hitAnim = "TopHit";
					}

					if (wasHit)
					{
						rock.state        = RockHead::State::Stopped;
						rock.speed        = 0.0f;
						velocity.x        = 0.0f;
						velocity.y        = 0.0f;
						animState.current = hitAnim;
						Haptics::PulseRockImpact(gamepadHaptics);
					}
					else
					{
						// Quick acceleration up to the top speed set in the prefab.
						rock.speed = std::min(rock.speed + rock.acceleration * deltaTime, rock.maxSpeed);

						if (rock.axis == RockHead::Axis::Horizontal)
						{
							velocity.x = rock.speed * static_cast<float>(rock.direction);
							velocity.y = 0.0f;
						}
						else
						{
							velocity.y = rock.speed * static_cast<float>(rock.direction);
							velocity.x = 0.0f;
						}

						// Idle, with a blink every half second.
						rock.blinkTimer -= deltaTime;
						if (rock.blinkTimer <= 0.0f)
						{
							animState.current = "Blink";
							rock.blinkTimer   = RockHead::BlinkInterval;
						}
						else if (animState.current == "Blink" && registry.Has<Animation>(entity))
						{
							const Animation& anim = registry.Get<Animation>(entity);
							if (anim.playingState == "Blink" && anim.isFinished)
								animState.current = "Idle";
						}
					}
					break;
				}

				case RockHead::State::Stopped:
					velocity.x = 0.0f;
					velocity.y = 0.0f;

					// Once the wall-wasHit animation finishes, set off the other way.
					if (registry.Has<Animation>(entity))
					{
						const Animation& anim = registry.Get<Animation>(entity);
						if (IsHitAnim(anim.playingState) && anim.isFinished)
						{
							rock.direction    = -rock.direction;
							rock.state        = RockHead::State::Moving;
							rock.speed        = 0.0f;
							rock.blinkTimer   = RockHead::BlinkInterval;
							animState.current = "Idle";
						}
					}
					break;
				}
			});
	}
}
