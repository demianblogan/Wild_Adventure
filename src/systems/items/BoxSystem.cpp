#include "BoxSystem.h"

#include "components/render/Animation.h"
#include "components/render/AnimationState.h"
#include "components/items/Box.h"
#include "components/items/BoxHit.h"
#include "components/physics/Collider.h"
#include "components/physics/CollisionState.h"
#include "components/physics/Gravity.h"
#include "components/physics/PreviousTransform.h"
#include "components/physics/Transform.h"
#include "components/physics/Velocity.h"
#include "components/items/PickupDelay.h"
#include "graphics/ParticleSystem.h"
#include "core/Random.h"
#include "core/SceneLoader.h"
#include "core/ecs/Registry.h"
#include "audio/Mixer.h"

#include <vector>

namespace ECS
{
	namespace
	{
		constexpr float FruitGravity = 700.0f;
		constexpr float FruitMaxFallSpeed = 400.0f;
		constexpr float FruitColliderSize = 12.0f;
		constexpr float FruitEjectSpawnRise = 5.0f; // spawn a bit above the box's own position
		constexpr float FruitEjectSpeedVarianceMin = 0.8f; // randomizes eject velocity so a
		constexpr float FruitEjectSpeedVarianceMax = 1.2f; // multi-fruit break doesn't look uniform
		constexpr float FruitPickupDelay = 0.4f; // seconds before an ejected fruit can be collected
		constexpr int DebrisPieceCount = 4;
	}

	BoxSystem::BoxSystem(Registry& registry, SceneLoader& loader, ParticleSystem& particles, Audio::Mixer& mixer)
		: registry(registry)
		, loader(loader)
		, particles(particles)
		, mixer(mixer)
	{}

	void BoxSystem::EjectFruit(const std::string& fruitName, float x, float y, float ejectSpeedX, float ejectSpeedUp)
	{
		const Entity fruit = loader.SpawnFromPrefab(registry, "data/prefabs/" + fruitName + ".json");

		const float sign = (Random::Int(0, 1) == 0) ? -1.0f : 1.0f;

		Transform transform;
		transform.x = x;
		transform.y = y;
		registry.Add<Transform>(fruit, transform);

		Velocity velocity;
		velocity.x = sign * ejectSpeedX * Random::Float(FruitEjectSpeedVarianceMin, FruitEjectSpeedVarianceMax);
		velocity.y = -ejectSpeedUp * Random::Float(FruitEjectSpeedVarianceMin, FruitEjectSpeedVarianceMax);
		registry.Add<Velocity>(fruit, velocity);

		Gravity gravity;
		gravity.acceleration = FruitGravity;
		gravity.maxFallSpeed = FruitMaxFallSpeed;
		registry.Add<Gravity>(fruit, gravity);

		Collider collider;
		collider.width = FruitColliderSize;
		collider.height = FruitColliderSize;
		registry.Add<Collider>(fruit, collider);

		registry.Add<CollisionState>(fruit, {});
		registry.Add<PreviousTransform>(fruit, { x, y });
		registry.Add<PickupDelay>(fruit, { FruitPickupDelay });
	}

	void BoxSystem::Update()
	{
		std::vector<Entity> hits;
		registry.ForEach<Box, BoxHit>(
			[&hits](Entity entity, Box&, BoxHit&) { hits.push_back(entity); });

		for (const Entity entity : hits)
		{
			Box& box = registry.Get<Box>(entity);

			if (!box.isBreaking)
			{
				box.hitsTaken++;

				if (registry.Has<AnimationState>(entity))
					registry.Get<AnimationState>(entity).current = "Hit";

				if (!box.hitSound.empty())
					mixer.PlaySound(box.hitSound);

				if (box.hitsTaken >= box.hitsToBreak)
					box.isBreaking = true;

				// One fruit per hit (sturdy wood box).
				if (box.hasFruitDropPerHit)
				{
					const int index = box.hitsTaken - 1;
					if (index >= 0 && index < static_cast<int>(box.fruits.size()))
					{
						const std::string fruitName = box.fruits[index];
						const Transform& boxTransform = registry.Get<Transform>(entity);
						const float x = boxTransform.x;
						const float y = boxTransform.y - FruitEjectSpawnRise;
						const float ejectX = box.ejectSpeedX;
						const float ejectUp = box.ejectSpeedUp;

						EjectFruit(fruitName, x, y, ejectX, ejectUp);
					}
				}
			}

			registry.RemoveFrom<BoxHit>(entity);
		}

		// Break or return to Idle once the Hit animation finishes.
		std::vector<Entity> toBreak;

		registry.ForEach<Box, AnimationState, Animation>(
			[&toBreak](Entity entity, Box& box, AnimationState& state, Animation& animation)
			{
				if (state.current == "Hit" && !animation.data.isLooping
					&& animation.currentFrame >= animation.data.frameCount - 1)
				{
					if (box.isBreaking)
						toBreak.push_back(entity);
					else
						state.current = "Idle";
				}
			});

		for (const Entity entity : toBreak)
		{
			Box& box = registry.Get<Box>(entity);

			// All fruits at once on break (metal box).
			if (!box.hasFruitDropPerHit)
			{
				const std::vector<std::string> fruits = box.fruits;
				const Transform& boxTransform = registry.Get<Transform>(entity);
				const float x = boxTransform.x;
				const float y = boxTransform.y - FruitEjectSpawnRise;
				const float ejectX = box.ejectSpeedX;
				const float ejectUp = box.ejectSpeedUp;

				for (const std::string& fruitName : fruits)
					EjectFruit(fruitName, x, y, ejectX, ejectUp);
			}

			if (!box.debrisTexture.empty())
			{
				const Transform& debrisTransform = registry.Get<Transform>(entity);
				particles.EmitDebris({ debrisTransform.x, debrisTransform.y - FruitEjectSpawnRise }, box.debrisTexture, DebrisPieceCount);
			}

			if (!box.breakSound.empty())
				mixer.PlaySound(box.breakSound);

			registry.DestroyEntity(entity);
		}
	}
}