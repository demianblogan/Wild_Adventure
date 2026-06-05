#include "DataLoader.h"

#include "components/Animation.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Collider.h"
#include "components/CollisionState.h"
#include "components/Gravity.h"
#include "components/Player.h"
#include "components/Facing.h"
#include "components/Patrol.h"
#include "components/PreviousTransform.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "core/ecs/Registry.h"
#include "components/Jump.h"
#include "components/WallSlide.h"
#include "components/Hazard.h"
#include "components/Health.h"
#include "components/Hitbox.h"

#include <fstream>
#include <stdexcept>

DataLoader::DataLoader()
{
	RegisterLoaders();
}

void DataLoader::RegisterLoaders()
{
	loaders["Transform"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Transform transform;
			transform.x = data.at("x");
			transform.y = data.at("y");
			registry.Add<ECS::Transform>(entity, transform);
		};

	loaders["Velocity"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Velocity velocity;
			velocity.x = data.at("x");
			velocity.y = data.at("y");
			registry.Add<ECS::Velocity>(entity, velocity);
		};

	loaders["Sprite"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Sprite sprite;
			sprite.textureName = data.at("textureName");
			registry.Add<ECS::Sprite>(entity, sprite);
		};

	loaders["AnimationSet"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::AnimationSet set;

			for (const auto& [stateName, animationData] : data.items())
			{
				ECS::AnimationData animation;
				animation.textureName = animationData.at("textureName");
				animation.frameCount = animationData.at("frameCount");
				animation.frameDuration = animationData.at("frameDuration");
				animation.isLooping = animationData.at("isLooping");
				animation.isReversed = animationData.value("reversed", false);

				set.animations[stateName] = animation;
			}

			registry.Add<ECS::AnimationSet>(entity, set);
		};

	loaders["AnimationState"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::AnimationState state;
			state.current = data.at("current");
			registry.Add<ECS::AnimationState>(entity, state);
		};

	loaders["Facing"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Facing facing;
			facing.isLookingRight = data.at("isLookingRight");
			facing.isTextureRight = data.at("isTextureRight");
			registry.Add<ECS::Facing>(entity, facing);
		};

	loaders["Patrol"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Patrol patrol;

			const std::string axis = data.at("axis");
			patrol.axis = (axis == "Vertical") ? ECS::Patrol::Axis::Vertical : ECS::Patrol::Axis::Horizontal;

			patrol.min = data.at("min");
			patrol.max = data.at("max");
			patrol.speed = data.at("speed");

			registry.Add<ECS::Patrol>(entity, patrol);
		};

	loaders["Collider"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Collider collider;
			collider.width = data.at("width");
			collider.height = data.at("height");
			registry.Add<ECS::Collider>(entity, collider);
		};

	loaders["Gravity"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Gravity gravity;
			gravity.acceleration = data.at("acceleration");
			gravity.maxFallSpeed = data.at("maxFallSpeed");
			registry.Add<ECS::Gravity>(entity, gravity);
		};

	loaders["Player"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Player player;
			player.moveSpeed = data.at("moveSpeed");
			player.acceleration = data.at("acceleration");
			player.deceleration = data.at("deceleration");
			registry.Add<ECS::Player>(entity, player);
		};

	loaders["Jump"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Jump jump;
			jump.jumpSpeed = data.at("jumpSpeed");
			jump.maxJumps = data.at("maxJumps");
			jump.jumpsRemaining = jump.maxJumps;
			registry.Add<ECS::Jump>(entity, jump);
		};

	loaders["Jump"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Jump jump;
			jump.jumpSpeed = data.at("jumpSpeed");
			jump.maxJumps = data.at("maxJumps");
			jump.jumpsRemaining = jump.maxJumps;
			jump.wallJumpPushX = data.value("wallJumpPushX", 0.0f);
			registry.Add<ECS::Jump>(entity, jump);
		};

	loaders["WallSlide"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::WallSlide wallSlide;
			wallSlide.slideSpeed = data.at("slideSpeed");
			registry.Add<ECS::WallSlide>(entity, wallSlide);
		};

	loaders["Health"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Health health;
			health.maximum = data.at("maximum");
			health.current = health.maximum;
			health.invulnerabilityDuration = data.at("invulnerabilityDuration");
			health.hitStunDuration = data.at("hitStunDuration");
			health.knockbackSpeed = data.at("knockbackSpeed");
			registry.Add<ECS::Health>(entity, health);
		};

	loaders["Hazard"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Hazard hazard;
			hazard.damage = data.at("damage");
			registry.Add<ECS::Hazard>(entity, hazard);
		};

	loaders["Hitbox"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Hitbox hitbox;
			hitbox.width = data.at("width");
			hitbox.height = data.at("height");
			registry.Add<ECS::Hitbox>(entity, hitbox);
		};
}

void DataLoader::AddImpliedComponents(ECS::Registry& registry, const std::vector<ECS::Entity>& entities)
{
	for (const ECS::Entity entity : entities)
	{
		if (registry.Has<ECS::Patrol>(entity) && !registry.Has<ECS::Velocity>(entity))
			registry.Add<ECS::Velocity>(entity, {});

		if (registry.Has<ECS::Gravity>(entity) && !registry.Has<ECS::Velocity>(entity))
			registry.Add<ECS::Velocity>(entity, {});

		if (registry.Has<ECS::Velocity>(entity) && registry.Has<ECS::Transform>(entity)
			&& !registry.Has<ECS::PreviousTransform>(entity))
		{
			const ECS::Transform& transform = registry.Get<ECS::Transform>(entity);
			registry.Add<ECS::PreviousTransform>(entity, { transform.x, transform.y });
		}

		if (registry.Has<ECS::Collider>(entity) && !registry.Has<ECS::CollisionState>(entity))
			registry.Add<ECS::CollisionState>(entity, {});

		if (registry.Has<ECS::AnimationSet>(entity) && !registry.Has<ECS::Animation>(entity))
			registry.Add<ECS::Animation>(entity, {});
	}
}

ECS::Entity DataLoader::LoadEntity(ECS::Registry& registry, const nlohmann::json& entityJson)
{
	const ECS::Entity entity = registry.CreateEntity();

	for (const auto& [componentName, componentData] : entityJson.items())
	{
		const auto found = loaders.find(componentName);

		if (found == loaders.end())
			throw std::runtime_error("Unknown component in data: " + componentName);

		found->second(registry, entity, componentData);
	}

	return entity;
}

ECS::Entity DataLoader::LoadEntityFromFile(ECS::Registry& registry, const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
		throw std::runtime_error("Could not open data file: " + path);

	const nlohmann::json entityJson = nlohmann::json::parse(file);

	return LoadEntity(registry, entityJson);
}

ECS::Entity DataLoader::LoadPrefabbedEntity(ECS::Registry& registry, const nlohmann::json& entry)
{
	nlohmann::json overrides = entry;

	const std::string prefabPath = overrides.at("prefab");
	overrides.erase("prefab");

	std::ifstream prefabFile(prefabPath);

	if (!prefabFile.is_open())
		throw std::runtime_error("Could not open prefab file: " + prefabPath);

	nlohmann::json merged = nlohmann::json::parse(prefabFile);
	merged.merge_patch(overrides);

	return LoadEntity(registry, merged);
}

std::vector<ECS::Entity> DataLoader::LoadScene(ECS::Registry& registry, const std::string& scenePath)
{
	std::ifstream sceneFile(scenePath);

	if (!sceneFile.is_open())
		throw std::runtime_error("Could not open scene file: " + scenePath);

	const nlohmann::json sceneJson = nlohmann::json::parse(sceneFile);

	std::vector<ECS::Entity> createdEntities;

	for (const auto& entityJson : sceneJson.at("entities"))
		createdEntities.push_back(LoadPrefabbedEntity(registry, entityJson));

	AddImpliedComponents(registry, createdEntities);

	return createdEntities;
}

std::vector<ECS::Entity> DataLoader::LoadSceneFromMap(ECS::Registry& registry, const std::string& mapPath)
{
	std::ifstream mapFile(mapPath);

	if (!mapFile.is_open())
		throw std::runtime_error("Could not open map file: " + mapPath);

	const nlohmann::json mapJson = nlohmann::json::parse(mapFile);

	std::vector<ECS::Entity> createdEntities;

	for (const auto& layer : mapJson.at("layers"))
	{
		if (layer.value("type", std::string()) != "objectgroup")
			continue;

		for (const auto& object : layer.at("objects"))
		{
			const std::string className = object.value("type", std::string());

			// Skip non-entity markers (e.g. the camera guide): no class, or not a point.
			if (className.empty() || !object.value("point", false))
				continue;

			const float x = object.at("x");
			const float y = object.at("y");

			nlohmann::json entry;
			entry["prefab"] = "data/prefabs/" + className + ".json";
			entry["Transform"]["x"] = x;
			entry["Transform"]["y"] = y;

			// patrolRange custom property -> Patrol bounds. Axis and speed stay from the prefab.
			if (object.contains("properties"))
			{
				for (const auto& property : object["properties"])
				{
					if (property.value("name", std::string()) == "patrolRange")
					{
						const float range = property.at("value");
						entry["Patrol"]["min"] = x - range;
						entry["Patrol"]["max"] = x + range;
					}
				}
			}

			createdEntities.push_back(LoadPrefabbedEntity(registry, entry));
		}
	}

	AddImpliedComponents(registry, createdEntities);

	return createdEntities;
}