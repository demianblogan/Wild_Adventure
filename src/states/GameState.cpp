#include "GameState.h"

#include "Context.h"
#include "components/Box.h"
#include "components/Collectible.h"
#include "components/CollisionState.h"
#include "components/Health.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "components/Animation.h"
#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/Frozen.h"
#include "components/PreviousTransform.h"
#include "components/Solid.h"
#include "components/AnimationSet.h"
#include "components/StartPlatform.h"	
#include "components/Finish.h"
#include "components/Checkpoint.h"
#include "components/Hitbox.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "core/Input.h"
#include "core/ecs/Registry.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"
#include "ui/Label.h"
#include "audio/Mixer.h"
#include "states/MenuState.h"
#include "states/PauseState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <memory>
#include <iostream> //DEBUG

namespace
{
	// Each block of five levels shares one music track. New tracks are registered in
	// data/audio.json; the level-number ranges are wired here.
	const std::string& LevelMusicTrack(int levelNumber)
	{
		static const std::string tracks[] =
		{
			"level_music_1_5",
			"level_music_6_10",
			"level_music_11_15",
			"level_music_16_20"
		};

		int index = (levelNumber - 1) / 5;
		if (index < 0)
			index = 0;
		if (index > 3)
			index = 3;

		return tracks[index];
	}
}

GameState::GameState(Context& context, const std::string& levelPath, int levelNumber,
	std::optional<sf::Vector2f> respawnAt, int initialScore,
	std::optional<ProgressSnapshot> progressSnapshot)
	: State(context)
	, background(context.resources)
	, particles(context.resources)
	, confetti(context.resources)
	, inputSystem(registry, context.input)
	, jumpSystem(registry)
	, damageSystem(registry)
	, deathSystem(registry)
	, patrolSystem(registry)
	, physicsSystem(registry, tilemap)
	, boxSystem(registry, sceneLoader, particles, context.audioMixer)
	, movementSystem(registry)
	, pickupSystem(registry, score)
	, animationSystem(registry)
	, playerAnimationSystem(registry)
	, renderSystem(registry, context.resources, context.virtualScreen.GetRenderTarget())
	, hudInterface(context.virtualScreen)
	, hudLoader(context.resources)
	, levelPath(levelPath)
	, levelNumber(levelNumber)
	, respawnOverride(respawnAt)
	, score(initialScore)
{
	Resources& resources = context.resources;

	if (!resources.fonts.Has("main"))
	{
		resources.fonts.Load("main", "assets/fonts/main.ttf");
		resources.fonts.Get("main").setSmooth(false);
	}

	resources.LoadTexturesFromFile("data/levels/game_textures.json");
	particles.LoadConfig("data/particles.json");

	background.SetTexture("blue");
	background.SetDirection(AnimatedBackground::Direction::Right);
	background.SetSpeed(16.0f);

	tilemap = LoadTilemap(levelPath, "terrain", 22);
	particles.SetTilemap(tilemap);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);

	fallLimit = worldSize.y + 64.0f;

	sceneLoader.LoadSceneFromMap(registry, levelPath);

	// Restore the world to the state it was in at the last checkpoint: remove
	// collectibles that had already been picked up and boxes that had been broken.
	if (progressSnapshot.has_value())
	{
		auto positionMatch = [](const sf::Vector2f& a, const sf::Vector2f& b)
		{
			return std::abs(a.x - b.x) < 2.0f && std::abs(a.y - b.y) < 2.0f;
		};

		std::vector<ECS::Entity> toDestroy;

		registry.ForEach<ECS::Collectible, ECS::Transform>(
			[&](ECS::Entity entity, ECS::Collectible&, ECS::Transform& t)
			{
				const sf::Vector2f pos{ t.x, t.y };
				const bool wasAlive = std::ranges::any_of(progressSnapshot->aliveCollectibles,
					[&](const sf::Vector2f& p) { return positionMatch(pos, p); });
				if (!wasAlive)
					toDestroy.push_back(entity);
			});

		registry.ForEach<ECS::Box, ECS::Transform>(
			[&](ECS::Entity entity, ECS::Box&, ECS::Transform& t)
			{
				const sf::Vector2f pos{ t.x, t.y };
				const bool wasAlive = std::ranges::any_of(progressSnapshot->aliveBoxes,
					[&](const sf::Vector2f& p) { return positionMatch(pos, p); });
				if (!wasAlive)
					toDestroy.push_back(entity);
			});

		for (ECS::Entity e : toDestroy)
			registry.DestroyEntity(e);
	}

	// The hero spawns above the start platform, so a level only needs a single
	// "start" marker in the "marks" object layer.
	registry.ForEach<ECS::StartPlatform>(
		[this](ECS::Entity entity, ECS::StartPlatform&) { startPlatformEntity = entity; });

	registry.ForEach<ECS::Finish>(
		[this](ECS::Entity entity, ECS::Finish&) { finishEntity = entity; });

	if (startPlatformEntity != ECS::INVALID_ENTITY || respawnOverride.has_value())
	{
		playerEntity = sceneLoader.SpawnFromPrefab(registry, "data/prefabs/player.json");

		registry.Add<ECS::Transform>(playerEntity, {});
		registry.Add<ECS::PreviousTransform>(playerEntity, {});

		float feetX = 0.0f;
		float feetY = 0.0f;

		if (respawnOverride.has_value())
		{
			feetX = respawnOverride->x;
			feetY = respawnOverride->y;
		}
		else
		{
			const ECS::Transform startTransform = registry.Get<ECS::Transform>(startPlatformEntity);
			const ECS::Solid startSolid = registry.Get<ECS::Solid>(startPlatformEntity);
			feetX = startTransform.x + startSolid.offsetX;
			feetY = startTransform.y + startSolid.offsetY - startSolid.height; // platform surface
		}

		respawnPoint = { feetX, feetY };

		const float airX = feetX;
		const float airY = feetY - APPEAR_HEIGHT;

		ECS::Transform& playerTransform = registry.Get<ECS::Transform>(playerEntity);
		playerTransform.x = airX;
		playerTransform.y = airY;

		ECS::PreviousTransform& previous = registry.Get<ECS::PreviousTransform>(playerEntity);
		previous.x = airX;
		previous.y = airY;

		const ECS::Health& health = registry.Get<ECS::Health>(playerEntity);
		maxHearts = health.maximum;
		displayedHealth = health.maximum;
		previousPlayerHealth = health.maximum;

		registry.Add<ECS::Frozen>(playerEntity, {});
		camera.SnapTo({ airX, airY });
	}

	hudInterface.SetContent(hudLoader.LoadFromFile("data/ui/hud.json"));
	UpdateScoreLabel();

	context.audioMixer.PlayMusic(LevelMusicTrack(levelNumber));

	transition.StartReveal();
}

void GameState::HandleEvent(const sf::Event& event)
{}

void GameState::UpdateLevelFlow(float deltaTime)
{
	if (playerEntity == ECS::INVALID_ENTITY)
		return;

	if (levelPhase == LevelPhase::Revealing)
	{
		// Wait for the wipe to clear, then spawn the appear effect so it is visible.
		if (transition.GetMode() != Transition::Mode::Idle)
			return;

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

		context.audioMixer.PlaySound("player_appear");

		levelPhase = LevelPhase::Appearing;
		return;
	}

	if (levelPhase == LevelPhase::Appearing)
	{
		const bool finished = appearEffectEntity != ECS::INVALID_ENTITY
			&& registry.Has<ECS::Animation>(appearEffectEntity)
			&& registry.Get<ECS::Animation>(appearEffectEntity).isFinished;

		if (finished)
		{
			registry.DestroyEntity(appearEffectEntity);
			appearEffectEntity = ECS::INVALID_ENTITY;

			if (registry.Has<ECS::Frozen>(playerEntity))
				registry.RemoveFrom<ECS::Frozen>(playerEntity);

			levelPhase = LevelPhase::Playing;
		}

		return;
	}

	if (levelPhase == LevelPhase::Finishing)
	{
		// The hero bounced off the cup and rises for a moment, then vanishes mid-air.
		finishTimer -= deltaTime;
		if (finishTimer > 0.0f)
			return;

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

		levelPhase = LevelPhase::Disappearing;
		return;
	}

	if (levelPhase == LevelPhase::Disappearing)
	{
		const bool finished = disappearEffectEntity != ECS::INVALID_ENTITY
			&& registry.Has<ECS::Animation>(disappearEffectEntity)
			&& registry.Get<ECS::Animation>(disappearEffectEntity).isFinished;

		if (finished)
		{
			registry.DestroyEntity(disappearEffectEntity);
			disappearEffectEntity = ECS::INVALID_ENTITY;

			transition.StartCover();
			isCompleting = true;
			levelPhase = LevelPhase::Complete;
		}

		return;
	}

	if (levelPhase == LevelPhase::Complete)
		return;

	// Playing.

	// The start platform plays its "moving" animation once the hero lands on it,
	// then returns to idle.
	if (startPlatformEntity != ECS::INVALID_ENTITY)
	{
		if (!startMovingPlayed && IsPlayerOnStartPlatform())
		{
			registry.Get<ECS::AnimationState>(startPlatformEntity).current = "moving";
			startMovingPlayed = true;
			confetti.Emit(PlayerCenter());
		}
		else if (startMovingPlayed)
		{
			const ECS::Animation& animation = registry.Get<ECS::Animation>(startPlatformEntity);
			if (animation.playingState == "moving" && animation.isFinished)
				registry.Get<ECS::AnimationState>(startPlatformEntity).current = "idle";
		}
	}

	UpdateCheckpoints();

	// Touching the top of the finish cup bounces the hero and ends the level.
	if (finishEntity != ECS::INVALID_ENTITY && IsPlayerOnFinish())
	{
		registry.Get<ECS::AnimationState>(finishEntity).current = "pressed";
		context.audioMixer.PlaySound("level_complete");
		confetti.Emit(PlayerCenter());

		// The Solid's bounceSpeed already launched the hero upward this frame; he
		// rises for FINISH_RISE_TIME, then vanishes.
		finishTimer = FINISH_RISE_TIME;
		levelPhase = LevelPhase::Finishing;
	}
}

void GameState::UpdateCheckpoints()
{
	const ECS::Transform& player = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);

	const float playerHalf = collider.width / 2.0f;
	const float playerLeft = player.x - playerHalf;
	const float playerRight = player.x + playerHalf;
	const float playerTop = player.y - collider.height;
	const float playerBottom = player.y;

	// Touching an inactive checkpoint activates it: raise the flag, save the respawn
	// point and play the sound. Already-active checkpoints can't be re-triggered.
	registry.ForEach<ECS::Checkpoint, ECS::Transform, ECS::Hitbox, ECS::AnimationState>(
		[&](ECS::Entity, ECS::Checkpoint& checkpoint, ECS::Transform& transform,
			ECS::Hitbox& hitbox, ECS::AnimationState& state)
		{
			if (checkpoint.activated)
				return;

			const float halfWidth = hitbox.width / 2.0f;
			const float left = transform.x - halfWidth;
			const float right = transform.x + halfWidth;
			const float top = transform.y - hitbox.height;
			const float bottom = transform.y;

			const bool overlap = playerLeft < right && playerRight > left
				&& playerTop < bottom && playerBottom > top;

			if (!overlap)
				return;

			checkpoint.activated = true;
			state.current = "flag_out";
			respawnPoint = { transform.x, transform.y };
			context.audioMixer.PlaySound("checkpoint");
			confetti.Emit({ player.x, player.y - collider.height / 2.0f });

			// Freeze the score and snapshot all alive collectibles and unbroken boxes
			// so we can restore this exact state if the player dies here.
			checkpointScore = score;

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
			checkpointSnapshot = std::move(snap);
		});

	// Once the flag has finished raising, loop the idle waving animation.
	registry.ForEach<ECS::Checkpoint, ECS::Animation, ECS::AnimationState>(
		[](ECS::Entity, ECS::Checkpoint& checkpoint, ECS::Animation& animation,
			ECS::AnimationState& state)
		{
			if (checkpoint.activated && animation.playingState == "flag_out" && animation.isFinished)
				state.current = "flag_idle";
		});
}

bool GameState::IsPlayerOnFinish()
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

sf::Vector2f GameState::PlayerCenter()
{
	const ECS::Transform& transform = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);

	return { transform.x, transform.y - collider.height / 2.0f };
}

bool GameState::IsPlayerOnStartPlatform()
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

void GameState::UpdateScoreLabel()
{
	if (score == previousScore)
		return;

	if (UI::Element* element = hudInterface.FindByName("score"))
	{
		if (auto* label = dynamic_cast<UI::Label*>(element))
			label->SetText("Score: " + std::to_string(score));
	}

	previousScore = score;
}

void GameState::UpdateHearts(int currentHealth, float deltaTime)
{
	// A point was just lost: start blinking the rightmost shown heart.
	if (currentHealth < displayedHealth && blinkingHeart < 0)
	{
		blinkingHeart = currentHealth; // heart index that will disappear
		blinkTimer = HEART_BLINK_DURATION;
		displayedHealth = currentHealth;
	}

	bool blinkOn = true;
	if (blinkingHeart >= 0)
	{
		blinkTimer -= deltaTime;
		blinkOn = std::fmod(blinkTimer, 0.12f) < 0.06f; // fast on/off

		if (blinkTimer <= 0.0f)
			blinkingHeart = -1; // fully gone now
	}

	for (int i = 0; i < maxHearts; i++)
	{
		UI::Element* heart = hudInterface.FindByName("heart" + std::to_string(i));
		if (heart == nullptr)
			continue;

		if (i < displayedHealth)
			heart->isVisible = true;       // settled, alive
		else if (i == blinkingHeart)
			heart->isVisible = blinkOn;    // blinking out
		else
			heart->isVisible = false;      // gone
	}
}

void GameState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	// Decay the death flash here so it still fades while the restart wipe runs (the
	// blocks below return early once a death/finish is in progress).
	if (deathFlashTimer > 0.0f)
		deathFlashTimer -= deltaTime;

	if (isCompleting)
	{
		if (transition.GetMode() == Transition::Mode::Done)
		{
			context.stateMachine.Clear();
			context.stateMachine.Push(std::make_unique<MenuState>(context));
		}
		return;
	}

	if (isRestarting)
	{
		if (transition.GetMode() == Transition::Mode::Done)
		{
			context.stateMachine.Pop();
			context.stateMachine.Push(std::make_unique<GameState>(context, levelPath, levelNumber, respawnPoint, checkpointScore, checkpointSnapshot));
		}
		return;
	}

	if (levelPhase == LevelPhase::Playing && context.input.WasPressed(Action::Pause))
	{
		// Freeze interpolation so the level doesn't jitter while paused.
		camera.SnapTo(camera.GetRenderCenter(1.0f));
		registry.ForEach<ECS::Transform, ECS::PreviousTransform>(
			[](ECS::Entity, ECS::Transform& t, ECS::PreviousTransform& pt) { pt.x = t.x; pt.y = t.y; });

		context.stateMachine.Push(std::make_unique<PauseState>(context, levelPath, levelNumber));
		return;
	}

	inputSystem.Update(deltaTime);
	jumpSystem.Update();
	damageSystem.Update(deltaTime);
	deathSystem.Update(deltaTime);
	patrolSystem.Update();
	physicsSystem.Update(deltaTime);
	boxSystem.Update();
	movementSystem.Update(deltaTime);

	const int scoreBeforePickup = score;
	pickupSystem.Update(deltaTime);
	if (score > scoreBeforePickup)
		context.audioMixer.PlaySound("fruit_collect");

	playerAnimationSystem.Update();
	animationSystem.Update(deltaTime);

	UpdateLevelFlow(deltaTime);

	particles.Update(deltaTime);
	confetti.Update(deltaTime);
	background.Update(deltaTime);

	UpdateScoreLabel();
	hudInterface.Update(deltaTime);

	registry.ForEach<ECS::Player, ECS::Transform, ECS::Velocity, ECS::CollisionState, ECS::Jump, ECS::Health>(
		[this, deltaTime](ECS::Entity, ECS::Player&, ECS::Transform& transform, ECS::Velocity& velocity,
			ECS::CollisionState& collisionState, ECS::Jump& jump, ECS::Health& health)
		{
			UpdateHearts(health.current, deltaTime);

			const sf::Vector2f feet = { transform.x, transform.y };
			const bool onGround = collisionState.isOnGround;

			camera.MoveTo(feet);

			if (onGround && std::abs(velocity.x) > 5.0f)
			{
				runDustTimer -= deltaTime;
				if (runDustTimer <= 0.0f)
				{
					const int runDirection = (velocity.x > 0.0f) ? 1 : -1;
					const sf::Vector2f runEmit = { feet.x - runDirection * particles.GetRunBackOffset(), feet.y };
					particles.Emit("run", runEmit);
					runDustTimer = RUN_DUST_INTERVAL;
				}
			}
			else
			{
				runDustTimer = 0.0f;
			}

			// Looping wall-slide sound only while actually sliding down a wall.
			const bool isWallSliding = collisionState.isOnWall && !onGround && velocity.y > 0.0f;
			if (isWallSliding)
				context.audioMixer.StartLoop("player_wall_slide");
			else
				context.audioMixer.StopLoop("player_wall_slide");

			if (previousLockTimer <= 0.0f && jump.lockTimer > 0.0f)
			{
				const int pushDirection = (velocity.x > 0.0f) ? 1 : -1;
				particles.Emit("wall_jump", feet, pushDirection);
				context.audioMixer.PlaySound("player_jump");
			}
			else if (jump.jumpsRemaining < previousJumpsRemaining)
			{
				const bool isDoubleJump = (jump.jumpsRemaining == 0);
				particles.Emit("jump", feet);
				context.audioMixer.PlaySound(isDoubleJump ? "player_double_jump" : "player_jump");
			}

			if (!wasOnGround && onGround)
				particles.Emit("land", feet);

			wasOnGround = onGround;
			previousJumpsRemaining = jump.jumpsRemaining;
			previousLockTimer = jump.lockTimer;

			const bool fellIntoPit = transform.y > fallLimit;

			if (health.current < previousPlayerHealth && health.current > 0)
				context.audioMixer.PlaySound("player_hurt");
			previousPlayerHealth = health.current;

			if (!isRestarting && (health.current <= 0 || fellIntoPit) && !deathSoundPlayed)
			{
				context.audioMixer.PlaySound("player_death");
				deathSoundPlayed = true;
				deathFlashTimer = DEATH_FLASH_TIME;
			}

			if (fellIntoPit)
			{
				transition.StartCover();
				isRestarting = true;
			}
		});
}

void GameState::Render(float interpolationFactor)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	renderTarget.clear(sf::Color::Black);

	// Background in screen space.
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	background.Draw(renderTarget);

	// Death "lightning": briefly wash the background white, fading back to normal.
	if (deathFlashTimer > 0.0f)
	{
		const float intensity = deathFlashTimer / DEATH_FLASH_TIME;
		sf::RectangleShape flash({ static_cast<float>(VirtualScreen::WIDTH), static_cast<float>(VirtualScreen::HEIGHT) });
		flash.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(intensity * 255.0f)));
		renderTarget.draw(flash);
	}

	// World in camera space.
	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);

	particles.Draw(renderTarget);
	renderSystem.Render(interpolationFactor);
	confetti.Draw(renderTarget);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	hudInterface.Draw(renderTarget);
	transition.Draw(renderTarget);
}