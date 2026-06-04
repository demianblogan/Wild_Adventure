#include "GameState.h"

#include "Context.h"
#include "components/Player.h"
#include "components/Transform.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "core/ecs/Registry.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

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
{
	context.resources.LoadTexturesFromFile("data/levels/game_textures.json");

	tilemap = LoadTilemap(levelPath, "terrain", 22);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);

	sceneLoader.LoadSceneFromMap(registry, levelPath);

	// Centre the camera on the player at spawn.
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

	inputSystem.Update();
	jumpSystem.Update();
	patrolSystem.Update();
	physicsSystem.Update(deltaTime);
	movementSystem.Update(deltaTime);

	playerAnimationSystem.Update();
	animationSystem.Update(deltaTime);

	// Camera follows the player.
	registry.ForEach<ECS::Player, ECS::Transform>(
		[this](ECS::Entity, ECS::Player&, ECS::Transform& transform)
		{
			camera.MoveTo({ transform.x, transform.y });
		});
}

void GameState::Render(float interpolationFactor)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	renderTarget.clear(sf::Color(60, 140, 200)); // plain sky for now

	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);
	renderSystem.Render(interpolationFactor);

	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	transition.Draw(renderTarget);
}