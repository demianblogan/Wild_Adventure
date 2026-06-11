#include "MenuBackdrop.h"

#include "Context.h"
#include "core/Resources.h"
#include "core/VirtualScreen.h"
#include "tilemap/TilemapLoader.h"
#include "tilemap/TilemapRenderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

MenuBackdrop::MenuBackdrop(Context& context)
	: context(context)
	, patrolSystem(registry)
	, movementSystem(registry)
	, animationSystem(registry)
	, renderSystem(registry, context.resources, context.virtualScreen.GetRenderTarget())
	, background(context.resources)
{
	Resources& resources = context.resources;

	resources.LoadTexturesFromFile("data/levels/splash_textures.json");

	background.SetTexture("green");
	background.SetDirection(AnimatedBackground::Direction::Down);
	background.SetSpeed(16.0f);

	const nlohmann::json& mapJson = resources.GetMapJson("data/levels/splash_level_tilemap.tmj");

	tilemap = LoadTilemap(mapJson, "terrain", 22);

	const sf::Vector2f worldSize =
	{
		static_cast<float>(tilemap.GetWidth() * tilemap.tileSize),
		static_cast<float>(tilemap.GetHeight() * tilemap.tileSize)
	};
	camera.SetWorldSize(worldSize);

	const float halfViewWidth = VirtualScreen::WIDTH / 2.0f;
	panMinX = halfViewWidth;
	panMaxX = worldSize.x - halfViewWidth;
	panX = panMinX;
	panY = worldSize.y / 2.0f;
	camera.SnapTo({ panX, panY });

	sceneLoader.LoadSceneFromMap(registry, mapJson);
}

void MenuBackdrop::Update(float deltaTime)
{
	panX += panDirection * PAN_SPEED * deltaTime;

	if (panX >= panMaxX)
	{
		panX = panMaxX;
		panDirection = -1;
	}
	else if (panX <= panMinX)
	{
		panX = panMinX;
		panDirection = 1;
	}

	camera.MoveTo({ panX, panY });

	background.Update(deltaTime);

	patrolSystem.Update();
	movementSystem.Update(deltaTime);
	animationSystem.Update(deltaTime);
}

void MenuBackdrop::Render(float interpolationFactor)
{
	sf::RenderTarget& renderTarget = context.virtualScreen.GetRenderTarget();

	renderTarget.clear(sf::Color::Black);

	// Background in screen space.
	context.virtualScreen.SetCameraCenter(VirtualScreen::WIDTH / 2.0f, VirtualScreen::HEIGHT / 2.0f);
	background.Draw(renderTarget);

	// World in camera space.
	const sf::Vector2f worldCenter = camera.GetRenderCenter(interpolationFactor);
	context.virtualScreen.SetCameraCenter(worldCenter.x, worldCenter.y);

	DrawTilemap(tilemap, renderTarget, context.resources);
	renderSystem.Render(interpolationFactor);
}