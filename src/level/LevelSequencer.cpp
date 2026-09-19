#include "LevelSequencer.h"

#include "audio/Mixer.h"
#include "components/combat/Enemy.h"
#include "components/combat/EnemyDeath.h"
#include "components/combat/Health.h"
#include "components/items/Box.h"
#include "components/items/Checkpoint.h"
#include "components/items/Collectible.h"
#include "components/physics/Collider.h"
#include "components/physics/CollisionState.h"
#include "components/physics/Hitbox.h"
#include "components/physics/PreviousTransform.h"
#include "components/physics/Solid.h"
#include "components/physics/Transform.h"
#include "components/render/Animation.h"
#include "components/render/AnimationSet.h"
#include "components/render/AnimationState.h"
#include "components/tags/Frozen.h"
#include "core/AABB.h"
#include "core/Camera.h"
#include "core/SceneLoader.h"
#include "core/ecs/Registry.h"
#include "graphics/ConfettiSystem.h"
#include "graphics/Transition.h"
#include "screens/HUD.h"

#include <cmath>

LevelSequencer::LevelSequencer(ECS::Registry& registry, SceneLoader& sceneLoader, Camera& camera,
	ConfettiSystem& confetti, Audio::Mixer& audioMixer, Transition& transition, HUD& hud)
	: registry(registry)
	, sceneLoader(sceneLoader)
	, camera(camera)
	, confetti(confetti)
	, audioMixer(audioMixer)
	, transition(transition)
	, hud(hud)
{}

bool LevelSequencer::Update(float deltaTime, int score, int fruitsCollected, int enemiesKilled)
{
	if (playerEntity == ECS::InvalidEntity)
		return false;

	if (phase == Phase::Revealing)
	{
		// Wait for the wipe to clear, then spawn the appear effect so it is visible.
		if (transition.GetMode() != Transition::Mode::Idle)
			return false;

		// Copy the position by value: SpawnFromPrefab can reallocate the pools and
		// invalidate any reference into them.
		const float playerX = registry.Get<ECS::Transform>(playerEntity).x;
		const float playerY = registry.Get<ECS::Transform>(playerEntity).y;

		appearEffectEntity = sceneLoader.SpawnFromPrefab(registry, "data/prefabs/appear_effect.json");

		ECS::Transform& effectTransform = registry.Get<ECS::Transform>(appearEffectEntity);
		effectTransform.x = playerX;
		effectTransform.y = playerY;

		// Initialise the animation now so the very first rendered frame is one cell
		// wide; otherwise AnimationSystem only sets it next tick and frame 0 shows
		// the whole sheet.
		ECS::Animation& animation = registry.Get<ECS::Animation>(appearEffectEntity);
		const ECS::AnimationSet& set = registry.Get<ECS::AnimationSet>(appearEffectEntity);
		animation.data = set.animations.at("appear");
		animation.playingState = "appear";

		audioMixer.PlaySound("player_appear");

		hud.StartBanner();

		phase = Phase::Appearing;
		return false;
	}

	if (phase == Phase::Appearing)
	{
		const bool finished = appearEffectEntity != ECS::InvalidEntity
			&& registry.Has<ECS::Animation>(appearEffectEntity)
			&& registry.Get<ECS::Animation>(appearEffectEntity).isFinished;

		if (finished)
		{
			registry.DestroyEntity(appearEffectEntity);
			appearEffectEntity = ECS::InvalidEntity;

			if (registry.Has<ECS::Frozen>(playerEntity))
				registry.RemoveFrom<ECS::Frozen>(playerEntity);

			phase = Phase::Playing;
		}

		return false;
	}

	if (phase == Phase::Finishing)
	{
		// The hero bounced off the cup and rises for a moment, then vanishes mid-air.
		finishTimer -= deltaTime;
		if (finishTimer > 0.0f)
			return false;

		const float playerX = registry.Get<ECS::Transform>(playerEntity).x;
		const float playerY = registry.Get<ECS::Transform>(playerEntity).y;

		registry.Add<ECS::Frozen>(playerEntity, {}); // hide and hold the hero in place

		disappearEffectEntity = sceneLoader.SpawnFromPrefab(registry, "data/prefabs/disappear_effect.json");

		ECS::Transform& effectTransform = registry.Get<ECS::Transform>(disappearEffectEntity);
		effectTransform.x = playerX;
		effectTransform.y = playerY;

		ECS::Animation& animation = registry.Get<ECS::Animation>(disappearEffectEntity);
		const ECS::AnimationSet& set = registry.Get<ECS::AnimationSet>(disappearEffectEntity);
		animation.data = set.animations.at("disappear");
		animation.playingState = "disappear";

		phase = Phase::Disappearing;
		return false;
	}

	if (phase == Phase::Disappearing)
	{
		const bool finished = disappearEffectEntity != ECS::InvalidEntity
			&& registry.Has<ECS::Animation>(disappearEffectEntity)
			&& registry.Get<ECS::Animation>(disappearEffectEntity).isFinished;

		if (finished)
		{
			registry.DestroyEntity(disappearEffectEntity);
			disappearEffectEntity = ECS::InvalidEntity;

			// Freeze interpolation so the level doesn't jitter while the complete menu shows.
			camera.SnapTo(camera.GetRenderCenter(1.0f));
			registry.ForEach<ECS::Transform, ECS::PreviousTransform>(
				[](ECS::Entity, ECS::Transform& t, ECS::PreviousTransform& pt) { pt.x = t.x; pt.y = t.y; });

			phase = Phase::Complete;

			if (!hasShownLevelComplete)
			{
				hasShownLevelComplete = true;
				return true;
			}
		}

		return false;
	}

	if (phase == Phase::Complete)
		return false;

	// Playing.

	// The start platform plays its "moving" animation once the hero lands on it,
	// then returns to idle.
	if (startPlatformEntity != ECS::InvalidEntity)
	{
		if (!hasPlayedStartMoving && IsPlayerOnStartPlatform())
		{
			registry.Get<ECS::AnimationState>(startPlatformEntity).current = "moving";
			hasPlayedStartMoving = true;

			// Confetti bursts above the platform itself, not wherever the player is.
			const ECS::Transform& start = registry.Get<ECS::Transform>(startPlatformEntity);
			confetti.Emit({ start.x, start.y - ConfettiRise });

			camera.Shake(ShakeTouch);
		}
		else if (hasPlayedStartMoving)
		{
			const ECS::Animation& animation = registry.Get<ECS::Animation>(startPlatformEntity);
			if (animation.playingState == "moving" && animation.isFinished)
				registry.Get<ECS::AnimationState>(startPlatformEntity).current = "idle";
		}
	}

	UpdateCheckpoints(score, fruitsCollected, enemiesKilled);

	// Touching the top of the finish cup bounces the hero and ends the level.
	if (finishEntity != ECS::InvalidEntity && IsPlayerOnFinish())
	{
		registry.Get<ECS::AnimationState>(finishEntity).current = "pressed";
		audioMixer.PlaySound("level_complete");

		const ECS::Transform& finish = registry.Get<ECS::Transform>(finishEntity);
		confetti.Emit({ finish.x, finish.y - ConfettiRise });

		camera.Shake(ShakeTouch);

		// The Solid's bounceSpeed already launched the hero upward this frame; he
		// rises for FinishRiseTime, then vanishes.
		finishTimer = FinishRiseTime;
		phase = Phase::Finishing;
	}

	return false;
}

void LevelSequencer::UpdateCheckpoints(int score, int fruitsCollected, int enemiesKilled)
{
	const ECS::Transform& player = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);

	const AABB playerBox = FeetAABB(player.x, player.y, collider.width, collider.height);

	// Touching an inactive checkpoint activates it: raise the flag, save the respawn
	// point and play the sound. Already-active checkpoints can't be re-triggered.
	registry.ForEach<ECS::Checkpoint, ECS::Transform, ECS::Hitbox, ECS::AnimationState>(
		[&](ECS::Entity, ECS::Checkpoint& checkpoint, ECS::Transform& transform,
			ECS::Hitbox& hitbox, ECS::AnimationState& state)
		{
			if (checkpoint.isActivated)
				return;

			const AABB checkpointBox = FeetAABB(transform.x, transform.y, hitbox.width, hitbox.height);
			if (!playerBox.Overlaps(checkpointBox))
				return;

			checkpoint.isActivated = true;
			state.current = "flag_out";
			respawnPoint = { transform.x, transform.y };

			// Touching a checkpoint refills the hero's lives, so reaching one is a
			// genuine reprieve rather than just a respawn marker.
			if (registry.Has<ECS::Health>(playerEntity))
			{
				ECS::Health& health = registry.Get<ECS::Health>(playerEntity);
				health.current = health.maximum;
			}

			audioMixer.PlaySound("checkpoint");
			confetti.Emit({ transform.x, transform.y - ConfettiRise });
			camera.Shake(ShakeTouch);

			// Freeze the score and snapshot all alive collectibles and unbroken boxes
			// so we can restore this exact state if the player dies here.
			checkpointScore = score;
			checkpointFruitsCollected = fruitsCollected;
			checkpointEnemiesKilled = enemiesKilled;

			ProgressSnapshot snap;
			registry.ForEach<ECS::Collectible, ECS::Transform>(
				[&snap](ECS::Entity, ECS::Collectible&, ECS::Transform& t)
				{
					snap.aliveCollectibles.push_back({ t.x, t.y });
				});
			registry.ForEach<ECS::Box, ECS::Transform>(
				[&snap](ECS::Entity, ECS::Box& box, ECS::Transform& t)
				{
					if (!box.isBreaking)
						snap.aliveBoxes.push_back({ t.x, t.y });
				});
			registry.ForEach<ECS::Enemy, ECS::Health>(
				[&](ECS::Entity entity, ECS::Enemy& enemy, ECS::Health& health)
				{
					if (health.current > 0 && !registry.Has<ECS::EnemyDeath>(entity))
						snap.aliveEnemies.push_back({ enemy.spawnX, enemy.spawnY });
				});
			checkpointSnapshot = std::move(snap);
		});

	// Once the flag has finished raising, loop the idle waving animation.
	registry.ForEach<ECS::Checkpoint, ECS::Animation, ECS::AnimationState>(
		[](ECS::Entity, ECS::Checkpoint& checkpoint, ECS::Animation& animation,
			ECS::AnimationState& state)
		{
			if (checkpoint.isActivated && animation.playingState == "flag_out" && animation.isFinished)
				state.current = "flag_idle";
		});
}

bool LevelSequencer::IsPlayerOnFinish() const
{
	const ECS::Transform& player = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);
	const ECS::Transform& finish = registry.Get<ECS::Transform>(finishEntity);
	const ECS::Solid& solid = registry.Get<ECS::Solid>(finishEntity);

	const float playerHalf = collider.width / 2.0f;
	const float playerLeft = player.x - playerHalf;
	const float playerRight = player.x + playerHalf;
	const float playerBottom = player.y;

	const float solidHalf = solid.width / 2.0f;
	const float solidCenterX = finish.x + solid.offsetX;
	const float solidBottom = finish.y + solid.offsetY;
	const float solidLeft = solidCenterX - solidHalf;
	const float solidRight = solidCenterX + solidHalf;
	const float solidTop = solidBottom - solid.height;

	const bool horizontalOverlap = playerLeft < solidRight && playerRight > solidLeft;
	const bool restingOnTop = std::fabs(playerBottom - solidTop) < 4.0f;

	return horizontalOverlap && restingOnTop;
}

bool LevelSequencer::IsPlayerOnStartPlatform() const
{
	if (!registry.Has<ECS::CollisionState>(playerEntity)
		|| !registry.Get<ECS::CollisionState>(playerEntity).isOnGround)
		return false;

	const ECS::Transform& player = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);
	const ECS::Transform& platform = registry.Get<ECS::Transform>(startPlatformEntity);
	const ECS::Solid& solid = registry.Get<ECS::Solid>(startPlatformEntity);

	const float playerHalf = collider.width / 2.0f;
	const float playerLeft = player.x - playerHalf;
	const float playerRight = player.x + playerHalf;
	const float playerBottom = player.y;

	const float solidHalf = solid.width / 2.0f;
	const float solidCenterX = platform.x + solid.offsetX;
	const float solidBottom = platform.y + solid.offsetY;
	const float solidLeft = solidCenterX - solidHalf;
	const float solidRight = solidCenterX + solidHalf;
	const float solidTop = solidBottom - solid.height;

	const bool horizontalOverlap = playerLeft < solidRight && playerRight > solidLeft;
	const bool restingOnTop = std::fabs(playerBottom - solidTop) < 2.0f;

	return horizontalOverlap && restingOnTop;
}
