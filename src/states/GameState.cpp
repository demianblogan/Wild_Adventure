#include "GameState.h"

#include "Context.h"
#include "components/CollisionState.h"
#include "components/Health.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/Resources.h"
#include "core/StateMachine.h"
#include "core/VirtualScreen.h"
#include "core/ecs/Registry.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"
#include "ui/Label.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>
#include <memory>

GameState::GameState(Context& context, const std::string& levelPath)
	: State(context)
	, inputSystem(registry)
	, jumpSystem(registry)
	, damageSystem(registry)
	, deathSystem(registry)
	, patrolSystem(registry)
	, physicsSystem(registry, tilemap)
	, movementSystem(registry)
	, pickupSystem(registry, score)
	, animationSystem(registry)
	, playerAnimationSystem(registry)
	, renderSystem(registry, context.resources, context.virtualScreen.GetRenderTarget())
	, particles(context.resources)
	, hudInterface(context.virtualScreen)
	, hudLoader(context.resources)
	, levelPath(levelPath)
{
	Resources& resources = context.resources;

	if (!resources.fonts.Has("main"))
	{
		resources.fonts.Load("main", "assets/fonts/main.ttf");
		resources.fonts.Get("main").setSmooth(false);
	}

	resources.LoadTexturesFromFile("data/levels/game_textures.json");
	particles.LoadConfig("data/particles.json");

	tilemap = LoadTilemap(levelPath, "terrain", 22);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);

	fallLimit = worldSize.y + 64.0f;

	sceneLoader.LoadSceneFromMap(registry, levelPath);

	registry.ForEach<ECS::Player, ECS::Transform>(
		[this](ECS::Entity, ECS::Player&, ECS::Transform& transform)
		{
			camera.SnapTo({ transform.x, transform.y });
		});

	hudInterface.SetContent(hudLoader.LoadFromFile("data/ui/hud.json"));
	UpdateScoreLabel();

	transition.StartReveal();
}

void GameState::HandleEvent(const sf::Event& event)
{}

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

void GameState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	if (isRestarting)
	{
		if (transition.GetMode() == Transition::Mode::Done)
		{
			context.stateMachine.Pop();
			context.stateMachine.Push(std::make_unique<GameState>(context, levelPath));
		}
		return;
	}

	inputSystem.Update(deltaTime);
	jumpSystem.Update();
	damageSystem.Update(deltaTime);
	deathSystem.Update(deltaTime);
	patrolSystem.Update();
	physicsSystem.Update(deltaTime);
	movementSystem.Update(deltaTime);
	pickupSystem.Update();

	playerAnimationSystem.Update();
	animationSystem.Update(deltaTime);

	particles.Update(deltaTime);

	UpdateScoreLabel();
	hudInterface.Update(deltaTime);

	registry.ForEach<ECS::Player, ECS::Transform, ECS::Velocity, ECS::CollisionState, ECS::Jump, ECS::Health>(
		[this, deltaTime](ECS::Entity, ECS::Player&, ECS::Transform& transform, ECS::Velocity& velocity,
			ECS::CollisionState& collisionState, ECS::Jump& jump, ECS::Health& health)
		{
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

			if (previousLockTimer <= 0.0f && jump.lockTimer > 0.0f)
			{
				const int pushDirection = (velocity.x > 0.0f) ? 1 : -1;
				particles.Emit("wall_jump", feet, pushDirection);
			}
			else if (wasOnGround && !onGround && velocity.y < 0.0f)
			{
				particles.Emit("jump", feet);
			}
			else if (!wasOnGround && !onGround && jump.jumpsRemaining < previousJumpsRemaining)
			{
				particles.Emit("jump", feet);
			}

			if (!wasOnGround && onGround)
				particles.Emit("land", feet);

			wasOnGround = onGround;
			previousJumpsRemaining = jump.jumpsRemaining;
			previousLockTimer = jump.lockTimer;

			if (transform.y > fallLimit)
			{
				transition.StartCover();
				isRestarting = true;
			}
		});
}

void GameState::Render(float interpolationFactor)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	renderTarget.clear(sf::Color(60, 140, 200));

	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);
	particles.Draw(renderTarget);
	renderSystem.Render(interpolationFactor);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	hudInterface.Draw(renderTarget);
	transition.Draw(renderTarget);
}