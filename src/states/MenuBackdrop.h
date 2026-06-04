#pragma once

#include "core/Camera.h"
#include "core/DataLoader.h"
#include "core/ecs/Registry.h"
#include "graphics/AnimatedBackground.h"
#include "systems/AnimationSystem.h"
#include "systems/MovementSystem.h"
#include "systems/PatrolSystem.h"
#include "systems/RenderSystem.h"
#include "tilemap/Tilemap.h"

struct Context;

namespace sf
{
	class RenderTarget;
}

class MenuBackdrop
{
public:
	MenuBackdrop(Context& context);

	void Update(float deltaTime);
	void Render(float interpolationFactor);

private:
	Context& context;

	ECS::Registry registry;
	DataLoader sceneLoader;

	ECS::PatrolSystem patrolSystem;
	ECS::MovementSystem movementSystem;
	ECS::AnimationSystem animationSystem;
	ECS::RenderSystem renderSystem;

	Camera camera;
	Tilemap tilemap;
	AnimatedBackground background;

	float panX = 0.0f;
	float panY = 0.0f;
	float panMinX = 0.0f;
	float panMaxX = 0.0f;
	int panDirection = 1;

	static constexpr float PAN_SPEED = 60.0f; // pixels per second (1 virtual pixel per frame at 60 FPS)
};