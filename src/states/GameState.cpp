#include "GameState.h"

#include "Context.h"
#include "components/items/Box.h"
#include "components/items/Collectible.h"
#include "components/combat/Enemy.h"
#include "components/physics/CollisionState.h"
#include "components/combat/Health.h"
#include "components/physics/Jump.h"
#include "components/tags/Player.h"
#include "components/physics/Transform.h"
#include "components/physics/Velocity.h"
#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/physics/Collider.h"
#include "components/tags/Frozen.h"
#include "components/physics/PreviousTransform.h"
#include "components/physics/Solid.h"
#include "components/render/Sprite.h"
#include "components/items/StartPlatform.h"
#include "components/items/Finish.h"
#include "core/HapticCues.h"
#include "core/Campaign.h"
#include "core/Random.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "core/Input.h"
#include "core/ecs/Registry.h"
#include "level/LevelSetup.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"
#include "audio/Mixer.h"
#include "states/LevelCompleteState.h"
#include "states/PauseState.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <memory>

GameState::GameState(Context& context, const std::string& levelPath, int levelNumber,
	std::optional<sf::Vector2f> respawnAt, int initialScore,
	std::optional<ProgressSnapshot> progressSnapshot,
	int initialDeathCount, int initialFruitsCollected, int initialEnemiesKilled)
	: State(context)
	, background(context.resources)
	, particles(context.resources)
	, confetti(context.resources)
	, inputSystem(registry, context.input)
	, jumpSystem(registry)
	, damageSystem(registry)
	, deathSystem(registry)
	, patrolSystem(registry)
	, enemySystem(registry, score, context.audioMixer, enemiesKilled)
	, trunkSystem(registry)
	, plantSystem(registry)
	, beeSystem(registry)
	, chickenSystem(registry, particles)
	, snailSystem(registry, sceneLoader)
	, shellSystem(registry, context.audioMixer)
	, ghostSystem(registry, particles)
	, turtleSystem(registry)
	, groundPatrolSystem(registry, tilemap, particles)
	, enemyDeathSystem(registry)
	, physicsSystem(registry, tilemap)
	, rockHeadSystem(registry, tilemap, context.gamepadHaptics)
	, boxSystem(registry, sceneLoader, particles, context.audioMixer, context.gamepadHaptics)
	, trampolineSystem(registry, context.audioMixer, context.gamepadHaptics)
	, arrowSystem(registry, context.audioMixer, context.gamepadHaptics)
	, fireSystem(registry)
	, movementSystem(registry)
	, bulletSystem(registry, tilemap, particles)
	, pickupSystem(registry, score, fruitsCollected)
	, animationSystem(registry, &particles)
	, playerAnimationSystem(registry)
	, renderSystem(registry, context.resources, context.virtualScreen)
	, hud(context)
	, levelSequencer(registry, sceneLoader, camera, confetti, context.audioMixer, transition, hud, context.gamepadHaptics)
	, playerFeedback(camera, particles, context.audioMixer, context.gamepadHaptics)
	, levelPath(levelPath)
	, levelNumber(levelNumber)
	, respawnOverride(respawnAt)
	, score(initialScore)
	, deathCount(initialDeathCount)
	, fruitsCollected(initialFruitsCollected)
	, enemiesKilled(initialEnemiesKilled)
{
	Resources& resources = context.resources;

	if (!resources.fonts.Has("main"))
	{
		// Shares the button font's file: it is the only one of the three UI
		// fonts with Cyrillic glyphs, so "main" (used for most body text) has
		// to be backed by it too for Russian/Ukrainian to render at all.
		resources.fonts.Load("main", "assets/fonts/born2bsporty-fs.regular.otf");
		resources.fonts.Get("main").setSmooth(false);
	}

	resources.LoadTexturesFromFile("data/levels/game_textures.json");
	particles.LoadConfig("data/particles.json");

	LevelSetup::ApplyBackground(resources, background, levelNumber);
	lighting = LevelSetup::LoadLighting(resources, levelNumber);

	// The "Level X" banner greets a fresh level start, not a checkpoint respawn.
	hud.Build(levelNumber, !respawnOverride.has_value());
	hud.SetScore(score);

	// Parse the map once and reuse the parsed JSON for both the tilemap and the
	// scene; the cache also makes level restarts skip the parse entirely.
	const nlohmann::json& mapJSON = resources.GetMapJSON(levelPath);
	const std::string levelTheme = LevelSetup::Theme(mapJSON);
	context.virtualScreen.SetColorGrading(LoadColorGrading(levelTheme));

	// A water level is floatier and trickles ambient bubbles.
	if (levelTheme == "water")
	{
		isWaterLevel = true;
		physicsSystem.SetGravityScale(WaterGravityScale);
	}

	tilemap = LoadTilemap(mapJSON, "terrain", 22);
	particles.SetTilemap(tilemap);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);
	fallLimit = worldSize.y + 64.0f;

	InitScene(mapJSON, std::move(progressSnapshot));
	SpawnPlayer();

	context.audioMixer.PlayMusic(LevelSetup::MusicTrack(resources, levelNumber));
	transition.StartReveal();
}

void GameState::HandleEvent(const sf::Event&)
{}

void GameState::InitScene(const nlohmann::json& mapJSON, std::optional<ProgressSnapshot> progressSnapshot)
{
	sceneLoader.LoadSceneFromMap(registry, mapJSON);

	// Compute totals from the full scene before any checkpoint pruning.
	registry.ForEach<ECS::Collectible>([&](ECS::Entity, ECS::Collectible&) { maxFruits++; });
	registry.ForEach<ECS::Box>([&](ECS::Entity, ECS::Box& box)
		{ maxFruits += static_cast<int>(box.fruits.size()); });
	registry.ForEach<ECS::Enemy>([&](ECS::Entity, ECS::Enemy&) { maxEnemies++; });

	// Record each enemy's spawn position right after loading, before any movement occurs.
	// This is used by checkpoint snapshots to identify which enemies were alive.
	registry.ForEach<ECS::Enemy, ECS::Transform>(
		[](ECS::Entity, ECS::Enemy& enemy, ECS::Transform& t)
		{
			enemy.spawnX = t.x;
			enemy.spawnY = t.y;
		});

	// Restore the world to the state it was in at the last checkpoint: remove
	// collectibles that had already been picked up, boxes that had been broken,
	// and enemies that had already been killed.
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

		registry.ForEach<ECS::Enemy>(
			[&](ECS::Entity entity, ECS::Enemy& enemy)
			{
				const sf::Vector2f spawnPos{ enemy.spawnX, enemy.spawnY };
				const bool wasAlive = std::ranges::any_of(progressSnapshot->aliveEnemies,
					[&](const sf::Vector2f& p) { return positionMatch(spawnPos, p); });
				if (!wasAlive)
					toDestroy.push_back(entity);
			});

		for (ECS::Entity e : toDestroy)
			registry.DestroyEntity(e);
	}

	// The hero spawns above the start platform, so a level only needs a single
	// "start" marker in the "marks" object layer.
	registry.ForEach<ECS::StartPlatform>(
		[this](ECS::Entity entity, ECS::StartPlatform&)
		{
			startPlatformEntity = entity;
			levelSequencer.SetStartPlatform(entity);
		});

	registry.ForEach<ECS::Finish>(
		[this](ECS::Entity entity, ECS::Finish&) { levelSequencer.SetFinish(entity); });
}

void GameState::SpawnPlayer()
{
	if (startPlatformEntity == ECS::InvalidEntity && !respawnOverride.has_value())
		return;

	playerEntity = sceneLoader.SpawnFromPrefab(registry, "data/prefabs/player.json");
	LevelSetup::ApplySkin(registry, playerEntity, context.campaign.GetSelectedSkin());
	levelSequencer.SetPlayer(playerEntity);

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

	levelSequencer.SetRespawnPoint({ feetX, feetY });

	const float airX = feetX;
	const float airY = feetY - AppearHeight;

	ECS::Transform& playerTransform = registry.Get<ECS::Transform>(playerEntity);
	playerTransform.x = airX;
	playerTransform.y = airY;

	ECS::PreviousTransform& previous = registry.Get<ECS::PreviousTransform>(playerEntity);
	previous.x = airX;
	previous.y = airY;

	const ECS::Health& health = registry.Get<ECS::Health>(playerEntity);
	hud.SetMaxHearts(health.maximum);
	playerFeedback.ResetHealthBaseline(health.maximum);

	registry.Add<ECS::Frozen>(playerEntity, {});
	camera.SnapTo({ airX, airY });
}

bool GameState::IsPlayerOnDeathTile()
{
	const ECS::Transform& player = registry.Get<ECS::Transform>(playerEntity);
	const ECS::Collider& collider = registry.Get<ECS::Collider>(playerEntity);

	const float tileSize = static_cast<float>(tilemap.tileSize);
	const float halfWidth = collider.width / 2.0f;

	const int firstColumn = static_cast<int>(std::floor((player.x - halfWidth) / tileSize));
	const int lastColumn = static_cast<int>(std::floor((player.x + halfWidth - 0.001f) / tileSize));
	const int firstRow = static_cast<int>(std::floor((player.y - collider.height) / tileSize));
	const int lastRow = static_cast<int>(std::floor((player.y - 0.001f) / tileSize));

	for (int row = firstRow; row <= lastRow; row++)
	{
		for (int column = firstColumn; column <= lastColumn; column++)
		{
			if (tilemap.IsDeadly(column, row))
				return true;
		}
	}

	return false;
}

void GameState::Update(float deltaTime)
{
	transition.Update(deltaTime);
	hud.UpdateBanner(deltaTime);

	// Decay the death flash here so it still fades while the restart wipe runs (the
	// blocks below return early once a death/finish is in progress).
	if (deathFlashTimer > 0.0f)
		deathFlashTimer -= deltaTime;

	if (levelSequencer.GetPhase() == LevelSequencer::Phase::Complete)
		return;

	if (isRestarting)
	{
		if (transition.GetMode() == Transition::Mode::Done)
		{
			context.stateMachine.Pop();
			context.stateMachine.Push(std::make_unique<GameState>(context, levelPath, levelNumber,
				levelSequencer.GetRespawnPoint(), levelSequencer.GetCheckpointScore(),
				levelSequencer.GetCheckpointSnapshot(),
				deathCount, levelSequencer.GetCheckpointFruitsCollected(),
				levelSequencer.GetCheckpointEnemiesKilled()));
		}
		return;
	}

	if (levelSequencer.GetPhase() == LevelSequencer::Phase::Playing && context.input.WasPressed(Action::Pause))
	{
		// Freeze interpolation so the level doesn't jitter while paused.
		camera.SnapTo(camera.GetRenderCenter(1.0f));
		registry.ForEach<ECS::Transform, ECS::PreviousTransform>(
			[](ECS::Entity, ECS::Transform& t, ECS::PreviousTransform& pt) { pt.x = t.x; pt.y = t.y; });

		context.stateMachine.Push(std::make_unique<PauseState>(context, levelPath, levelNumber));
		return;
	}

	// Hit stop: hold the whole world still for a few steps. Sounds keep playing.
	if (hitStopTimer > 0.0f)
	{
		if (!isHitStopFrozen)
		{
			// Pin interpolation once so the frozen frames show one still picture.
			camera.SnapTo(camera.GetRenderCenter(1.0f));
			registry.ForEach<ECS::Transform, ECS::PreviousTransform>(
				[](ECS::Entity, ECS::Transform& t, ECS::PreviousTransform& pt) { pt.x = t.x; pt.y = t.y; });
			isHitStopFrozen = true;
		}

		hitStopTimer -= deltaTime;

		if (hitStopTimer <= 0.0f)
			isHitStopFrozen = false;

		return;
	}

	inputSystem.Update(deltaTime);
	jumpSystem.Update();
	fireSystem.Update(deltaTime); // toggles the burn Hazard that DamageSystem then applies
	damageSystem.Update(deltaTime);
	deathSystem.Update(deltaTime);
	patrolSystem.Update();

	// A successful stomp (the only way enemies die) triggers a brief hit stop;
	// the freeze itself starts at the top of the next update step, so the bounce
	// impulse from this step still gets applied first.
	const int enemiesBeforeStomp = enemiesKilled;
	enemySystem.Update();
	if (enemiesKilled > enemiesBeforeStomp)
	{
		hitStopTimer = HitStopDuration;
		Haptics::PulseStomp(context.gamepadHaptics);
	}
	trunkSystem.Update(deltaTime);
	plantSystem.Update(deltaTime);
	beeSystem.Update(deltaTime);
	chickenSystem.Update(deltaTime);
	snailSystem.Update(deltaTime);
	shellSystem.Update(deltaTime);
	ghostSystem.Update(deltaTime);
	turtleSystem.Update(deltaTime);
	groundPatrolSystem.Update(deltaTime);
	enemyDeathSystem.Update(deltaTime, fallLimit);
	arrowSystem.Update();
	physicsSystem.Update(deltaTime);
	rockHeadSystem.Update(deltaTime);
	boxSystem.Update();
	trampolineSystem.Update();
	movementSystem.Update(deltaTime);
	bulletSystem.Update(deltaTime);

	const int scoreBeforePickup = score;
	pickupSystem.Update(deltaTime);
	if (score > scoreBeforePickup)
	{
		context.audioMixer.PlaySound("fruit_collect");
		Haptics::PulseCollect(context.gamepadHaptics);
	}

	playerAnimationSystem.Update();
	animationSystem.Update(deltaTime);

	const bool levelJustCompleted = levelSequencer.Update(deltaTime, score, fruitsCollected, enemiesKilled);
	if (levelJustCompleted)
	{
		context.stateMachine.Push(std::make_unique<LevelCompleteState>(
			context, levelPath, levelNumber,
			deathCount, fruitsCollected, maxFruits, enemiesKilled, maxEnemies));
	}

	camera.Update(deltaTime);

	// Ambient bubbles: spawn across the bottom of the view and let them rise.
	if (isWaterLevel)
	{
		bubbleTimer -= deltaTime;
		if (bubbleTimer <= 0.0f)
		{
			bubbleTimer = WaterBubbleInterval;

			const sf::Vector2f center = camera.GetRenderCenter(1.0f);
			const float x = center.x + Random::Float(-0.5f, 0.5f) * VirtualScreen::Width;
			const float y = center.y + Random::Float(-0.5f, 0.5f) * VirtualScreen::Height;
			particles.EmitBubble({ x, y });
		}
	}

	particles.Update(deltaTime);
	confetti.Update(deltaTime);
	background.Update(deltaTime);

	hud.SetScore(score);
	hud.Update(deltaTime);

	UpdatePlayer(deltaTime);
}

void GameState::UpdatePlayer(float deltaTime)
{
	registry.ForEach<ECS::Player, ECS::Transform, ECS::Velocity, ECS::CollisionState, ECS::Jump, ECS::Health>(
		[this, deltaTime](ECS::Entity, ECS::Player&, ECS::Transform& transform, ECS::Velocity& velocity,
			ECS::CollisionState& collisionState, ECS::Jump& jump, ECS::Health& health)
		{
			hud.UpdateHearts(health.current, deltaTime);

			const sf::Vector2f feet = { transform.x, transform.y };
			camera.MoveTo(feet);

			ECS::Sprite* sprite = registry.Has<ECS::Sprite>(playerEntity)
				? &registry.Get<ECS::Sprite>(playerEntity)
				: nullptr;
			playerFeedback.Update(deltaTime, feet, velocity, collisionState, jump, health, sprite);

			// Death tiles short-circuit the fall: pits on tall maps kill on touch
			// instead of after a long drop to the world's bottom edge.
			const bool fellIntoPit = transform.y > fallLimit || IsPlayerOnDeathTile();

			if (!isRestarting && (health.current <= 0 || fellIntoPit) && !hasPlayedDeathSound)
			{
				context.audioMixer.PlaySound("player_death");
				Haptics::PulseDeath(context.gamepadHaptics);
				hasPlayedDeathSound = true;
				deathFlashTimer = DeathFlashTime;
				deathCount++;
			}

			// A hero killed by damage tumbles down with collisions off; cap that
			// fall so the restart doesn't wait for the world's bottom on tall maps.
			if (health.current <= 0)
				deathFallTimer += deltaTime;

			if (fellIntoPit || deathFallTimer >= DeathFallTime)
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

	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);

	// Background fills the screen but is anchored to the world, so running
	// around never changes its apparent scroll speed.
	context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
	background.Draw(renderTarget, worldCenter);

	// Death "lightning": briefly wash the background white, fading back to normal.
	if (deathFlashTimer > 0.0f)
	{
		const float intensity = deathFlashTimer / DeathFlashTime;
		sf::RectangleShape flash({ static_cast<float>(VirtualScreen::Width), static_cast<float>(VirtualScreen::Height) });
		flash.setFillColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(intensity * 255.0f)));
		renderTarget.draw(flash);
	}

	// World in camera space.
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);

	particles.Draw(renderTarget);
	renderSystem.Render(interpolationFactor);
	confetti.Draw(renderTarget);

	// Bloom the glowing world objects (fruits, finish) before the HUD goes on.
	context.virtualScreen.CompositeGlow();

	// Cave levels: darkness outside the player's lamp circle, interpolated
	// like the sprites so the light never lags behind the player.
	if (lighting.isEnabled)
	{
		registry.ForEach<ECS::Player, ECS::Transform, ECS::PreviousTransform, ECS::Collider>(
			[&](ECS::Entity, ECS::Player&, ECS::Transform& transform,
				ECS::PreviousTransform& previous, ECS::Collider& collider)
			{
				const float x = previous.x + (transform.x - previous.x) * interpolationFactor;
				const float y = previous.y + (transform.y - previous.y) * interpolationFactor;

				lightOverlay.Draw(renderTarget, { x, y - collider.height / 2.0f },
					lighting.radius, lighting.darkness);
			});
	}

	context.virtualScreen.SetCameraCenter(VirtualScreen::Width / 2.0f, VirtualScreen::Height / 2.0f);
	hud.Draw(renderTarget);
	transition.Draw(renderTarget);
}
