#pragma once

#include "core/Camera.h"
#include "core/SceneLoader.h"
#include "core/ecs/Registry.h"
#include "graphics/AnimatedBackground.h"
#include "graphics/ParticleSystem.h"
#include "systems/core/AnimationSystem.h"
#include "systems/enemies/BeeSystem.h"
#include "systems/enemies/GhostSystem.h"
#include "systems/enemies/GroundPatrolSystem.h"
#include "systems/core/MovementSystem.h"
#include "systems/enemies/PatrolSystem.h"
#include "systems/core/PhysicsSystem.h"
#include "systems/core/RenderSystem.h"
#include "systems/enemies/TurtleSystem.h"
#include "tilemap/Tilemap.h"

struct Context;

namespace sf
{
	class RenderTarget;
}

// The menu's animated backdrop: a real game level (menu_background_level.tmj) running
// without a player, with the camera panning across it. It reuses the actual gameplay
// systems so its enemies, traps and props behave exactly as they do in-game.
class MenuBackdrop
{
public:
	MenuBackdrop(Context& context);

	void Update(float deltaTime);
	void Render(float interpolationFactor);

private:
	Context& context;

	ECS::Registry  registry;
	SceneLoader    sceneLoader;
	ParticleSystem particles;
	Tilemap        tilemap;

	ECS::PatrolSystem       patrolSystem;
	ECS::PhysicsSystem      physicsSystem;
	ECS::GroundPatrolSystem groundPatrolSystem;
	ECS::BeeSystem          beeSystem;
	ECS::GhostSystem        ghostSystem;
	ECS::TurtleSystem       turtleSystem;
	ECS::MovementSystem     movementSystem;
	ECS::AnimationSystem    animationSystem;
	ECS::RenderSystem       renderSystem;

	Camera camera;
	AnimatedBackground background;

	float panX = 0.0f;
	float panY = 0.0f;
	float panMinX = 0.0f;
	float panMaxX = 0.0f;
	int panDirection = 1;

	static constexpr float PAN_SPEED = 60.0f; // pixels per second (1 virtual pixel per frame at 60 FPS)
};
