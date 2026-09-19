#include "PlayerFeedbackController.h"

#include "audio/Mixer.h"
#include "components/physics/CollisionState.h"
#include "components/physics/Jump.h"
#include "components/combat/Health.h"
#include "components/physics/Velocity.h"
#include "components/render/Sprite.h"
#include "core/Camera.h"
#include "graphics/ParticleSystem.h"

#include <algorithm>
#include <cmath>

PlayerFeedbackController::PlayerFeedbackController(Camera& camera, ParticleSystem& particles, Audio::Mixer& audioMixer)
	: camera(camera)
	, particles(particles)
	, audioMixer(audioMixer)
{}

void PlayerFeedbackController::Update(float deltaTime, sf::Vector2f feet, const ECS::Velocity& velocity,
	const ECS::CollisionState& collisionState, const ECS::Jump& jump, const ECS::Health& health, ECS::Sprite* sprite)
{
	const bool onGround = collisionState.isOnGround;

	// Spring the squash scale back toward normal; the triggers below
	// re-deform it with fresh full values.
	const float squashReturn = std::min(1.0f, SquashReturnSpeed * deltaTime);
	squashX += (1.0f - squashX) * squashReturn;
	squashY += (1.0f - squashY) * squashReturn;

	if (onGround && std::abs(velocity.x) > 5.0f)
	{
		runDustTimer -= deltaTime;
		if (runDustTimer <= 0.0f)
		{
			const int runDirection = (velocity.x > 0.0f) ? 1 : -1;
			particles.EmitRunDust(feet, runDirection);
			runDustTimer = RunDustInterval;
		}
	}
	else
	{
		runDustTimer = 0.0f;
	}

	// Looping wall-slide sound only while actually sliding down a wall.
	const bool isWallSliding = collisionState.isOnWall && !onGround && velocity.y > 0.0f;
	if (isWallSliding)
		audioMixer.StartLoop("player_wall_slide");
	else
		audioMixer.StopLoop("player_wall_slide");

	if (previousLockTimer <= 0.0f && jump.lockTimer > 0.0f)
	{
		const int pushDirection = (velocity.x > 0.0f) ? 1 : -1;
		particles.Emit("wall_jump", feet, pushDirection);
		audioMixer.PlaySound("player_jump");
		squashX = SquashJump.x;
		squashY = SquashJump.y;
	}
	else if (jump.jumpsRemaining < previousJumpsRemaining)
	{
		const bool isDoubleJump = (jump.jumpsRemaining == 0);
		particles.Emit("jump", feet);
		audioMixer.PlaySound(isDoubleJump ? "player_double_jump" : "player_jump");
		squashX = SquashJump.x;
		squashY = SquashJump.y;
	}

	if (!wasOnGround && onGround)
	{
		particles.Emit("land", feet);
		squashX = SquashLand.x;
		squashY = SquashLand.y;
	}

	wasOnGround = onGround;
	previousJumpsRemaining = jump.jumpsRemaining;
	previousLockTimer = jump.lockTimer;

	if (health.current < previousPlayerHealth)
	{
		camera.Shake(ShakeHit);

		// Compress along the impact axis. The knockback applied by the
		// damage systems reveals it: side hits launch diagonally
		// (velocity.x != 0), hits from above/below push straight up or down.
		const bool sideHit = std::abs(velocity.x) > 1.0f;
		squashX = sideHit ? SquashHitSide.x : SquashHitVertical.x;
		squashY = sideHit ? SquashHitSide.y : SquashHitVertical.y;

		if (health.current > 0)
			audioMixer.PlaySound("player_hurt");
	}
	previousPlayerHealth = health.current;

	if (sprite != nullptr)
	{
		sprite->scaleX = squashX;
		sprite->scaleY = squashY;
	}
}
