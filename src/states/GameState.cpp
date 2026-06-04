#include "GameState.h"

#include "Context.h"
#include "components/CollisionState.h"
#include "components/Jump.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "core/ecs/Registry.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cmath>

GameState::GameState(Context& context, const std::string& levelPath)
	: State(context)
	, inputSystem(registry)
	, jumpSystem(registry)
	, patrolSystem(registry)
	, physicsSystem(registry, tilemap)
	, movementSystem(registry)
	, animationSystem(registry)
	, playerAnimationSystem(registry)
	, renderSystem(registry, context.resources, context.virtualScreen.GetRenderTarget())
	, particles(context.resources)
{
	context.resources.LoadTexturesFromFile("data/levels/game_textures.json");
	particles.LoadConfig("data/particles.json");
	tilemap = LoadTilemap(levelPath, "terrain", 22);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);

	sceneLoader.LoadSceneFromMap(registry, levelPath);

	registry.ForEach<ECS::Player, ECS::Transform>(
		[this](ECS::Entity, ECS::Player&, ECS::Transform& transform)
		{
			camera.SnapTo({ transform.x, transform.y });
		});

	transition.StartReveal();
}

void GameState::HandleEvent(const sf::Event& event)
{}

void GameState::Update(float deltaTime)
{
	transition.Update(deltaTime);

	inputSystem.Update(deltaTime);
	jumpSystem.Update();
	patrolSystem.Update();
	physicsSystem.Update(deltaTime);
	movementSystem.Update(deltaTime);

	playerAnimationSystem.Update();
	animationSystem.Update(deltaTime);

	particles.Update(deltaTime);

	registry.ForEach<ECS::Player, ECS::Transform, ECS::Velocity, ECS::CollisionState, ECS::Jump>(
		[this, deltaTime](ECS::Entity, ECS::Player&, ECS::Transform& transform, ECS::Velocity& velocity,
			ECS::CollisionState& collisionState, ECS::Jump& jump)
		{
			const sf::Vector2f feet = { transform.x, transform.y };
			const bool onGround = collisionState.isOnGround;

			camera.MoveTo(feet);

			// Run dust: a small puff behind the feet at a steady interval while running.
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

			// Wall jump: the control lock just switched on this frame.
			if (previousLockTimer <= 0.0f && jump.lockTimer > 0.0f)
			{
				const int pushDirection = (velocity.x > 0.0f) ? 1 : -1;
				particles.Emit("wall_jump", feet, pushDirection);
			}
			// Jump off the ground.
			else if (wasOnGround && !onGround && velocity.y < 0.0f)
			{
				particles.Emit("jump", feet);
			}
			// Mid-air (double) jump.
			else if (!wasOnGround && !onGround && jump.jumpsRemaining < previousJumpsRemaining)
			{
				particles.Emit("jump", feet);
			}

			// Landing.
			if (!wasOnGround && onGround)
				particles.Emit("land", feet);

			wasOnGround = onGround;
			previousJumpsRemaining = jump.jumpsRemaining;
			previousLockTimer = jump.lockTimer;
		});
}

void GameState::Render(float interpolationFactor)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	renderTarget.clear(sf::Color(60, 140, 200));

	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);
	particles.Draw(renderTarget);   // behind the entities
	renderSystem.Render(interpolationFactor);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	transition.Draw(renderTarget);
}