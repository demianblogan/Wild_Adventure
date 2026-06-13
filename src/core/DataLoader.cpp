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
#include "components/Jump.h"
#include "components/WallSlide.h"
#include "components/Hazard.h"
#include "components/Health.h"
#include "components/Hitbox.h"
#include "components/Collectible.h"
#include "components/Box.h"
#include "components/Solid.h"
#include "components/StartPlatform.h"
#include "components/Enemy.h"
#include "components/Finish.h"
#include "components/Checkpoint.h"
#include "components/GroundPatrol.h"
#include "components/ChickenAI.h"
#include "components/TrunkAI.h"
#include "components/PlantAI.h"
#include "components/BeeAI.h"
#include "components/Trampoline.h"
#include "core/ecs/Registry.h"

#include <functional>
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
			sprite.offsetX = data.value("offsetX", 0.0f);
			sprite.offsetY = data.value("offsetY", 0.0f);

			// "glow" is either true (white aura) or an [r, g, b] color.
			if (data.contains("glow"))
			{
				const nlohmann::json& glow = data.at("glow");

				if (glow.is_array())
				{
					sprite.glow = true;
					sprite.glowColor = sf::Color(glow.at(0), glow.at(1), glow.at(2));
				}
				else
				{
					sprite.glow = glow.get<bool>();
				}
			}

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
				animation.particlePreset = animationData.value("particlePreset", std::string());
				animation.particleFrame = animationData.value("particleFrame", -1);

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
			patrol.direction = data.value("direction", 1);

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

	loaders["Collectible"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Collectible collectible;
			collectible.points = data.at("points");
			registry.Add<ECS::Collectible>(entity, collectible);
		};

	loaders["Solid"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Solid solid;
			solid.width = data.at("width");
			solid.height = data.at("height");
			solid.bounceSpeed = data.value("bounceSpeed", 0.0f);
			solid.offsetX = data.value("offsetX", 0.0f);
			solid.offsetY = data.value("offsetY", 0.0f);
			registry.Add<ECS::Solid>(entity, solid);
		};

	loaders["StartPlatform"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::StartPlatform>(entity, {});
		};
	loaders["Checkpoint"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::Checkpoint>(entity, {});
		};
	loaders["Finish"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::Finish>(entity, {});
		};

	loaders["Enemy"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Enemy enemy;
			enemy.scoreValue = data.value("scoreValue", 0);
			registry.Add<ECS::Enemy>(entity, enemy);
		};

	loaders["TrunkAI"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::TrunkAI>(entity, {});
		};

	loaders["PlantAI"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::PlantAI>(entity, {});
		};

	loaders["BeeAI"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::BeeAI bee;
			bee.width  = data.at("width");
			bee.height = data.at("height");
			bee.speed  = data.value("speed", bee.speed);
			registry.Add<ECS::BeeAI>(entity, bee);
		};

	loaders["ChickenAI"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::ChickenAI chicken;
			chicken.visionWidth  = data.at("visionWidth");
			chicken.visionHeight = data.at("visionHeight");
			chicken.speed        = data.at("speed");
			registry.Add<ECS::ChickenAI>(entity, chicken);
		};

	loaders["Trampoline"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json&)
		{
			registry.Add<ECS::Trampoline>(entity, {});
		};

	loaders["GroundPatrol"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::GroundPatrol patrol;
			patrol.speed     = data.value("speed",     30.0f);
			patrol.direction = data.value("direction", -1);
			registry.Add<ECS::GroundPatrol>(entity, patrol);
		};

	loaders["Box"] = [](ECS::Registry& registry, ECS::Entity entity, const nlohmann::json& data)
		{
			ECS::Box box;
			box.hitsToBreak = data.at("hitsToBreak");
			box.dropFruitPerHit = data.at("dropFruitPerHit");

			for (const auto& fruit : data.at("fruits"))
				box.fruits.push_back(fruit.get<std::string>());

			box.ejectSpeedX = data.value("ejectSpeedX", 0.0f);
			box.ejectSpeedUp = data.value("ejectSpeedUp", 0.0f);

			box.debrisTexture = data.value("debrisTexture", std::string());

			box.hitSound = data.value("hitSound", std::string());
			box.breakSound = data.value("breakSound", std::string());

			registry.Add<ECS::Box>(entity, box);
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
		{
			ECS::Animation anim;

			if (registry.Has<ECS::AnimationState>(entity))
			{
				const ECS::AnimationSet& set   = registry.Get<ECS::AnimationSet>(entity);
				const std::string&       state = registry.Get<ECS::AnimationState>(entity).current;
				const auto               found = set.animations.find(state);

				if (found != set.animations.end())
				{
					anim.data         = found->second;
					anim.playingState = state;

					// Keep sprite in sync so the first Render is correct before
					// AnimationSystem ticks.
					if (registry.Has<ECS::Sprite>(entity))
						registry.Get<ECS::Sprite>(entity).textureName = anim.data.textureName;
				}
			}

			registry.Add<ECS::Animation>(entity, std::move(anim));
		}
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

const nlohmann::json& DataLoader::GetCachedJson(const std::string& path)
{
	const auto found = fileCache.find(path);

	if (found != fileCache.end())
		return found->second;

	std::ifstream file(path);

	if (!file.is_open())
		throw std::runtime_error("Could not open data file: " + path);

	return fileCache.emplace(path, nlohmann::json::parse(file)).first->second;
}

ECS::Entity DataLoader::LoadEntityFromFile(ECS::Registry& registry, const std::string& path)
{
	return LoadEntity(registry, GetCachedJson(path));
}

ECS::Entity DataLoader::SpawnFromPrefab(ECS::Registry& registry, const std::string& path)
{
	const ECS::Entity entity = LoadEntityFromFile(registry, path);
	AddImpliedComponents(registry, { entity });
	return entity;
}

ECS::Entity DataLoader::LoadPrefabbedEntity(ECS::Registry& registry, const nlohmann::json& entry)
{
	nlohmann::json overrides = entry;

	const std::string prefabPath = overrides.at("prefab");
	overrides.erase("prefab");

	nlohmann::json merged = GetCachedJson(prefabPath);
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

	return LoadSceneFromMap(registry, nlohmann::json::parse(mapFile));
}

std::vector<ECS::Entity> DataLoader::LoadSceneFromMap(ECS::Registry& registry, const nlohmann::json& mapJson)
{
	std::vector<ECS::Entity> createdEntities;

	// Recursive lambda: handles both flat objectgroups and Tiled group layers.
	// Group and layer offsets accumulate so objects land where Tiled shows them.
	std::function<void(const nlohmann::json&, float, float)> processLayers =
		[&](const nlohmann::json& layers, float parentOffsetX, float parentOffsetY)
	{
		for (const auto& layer : layers)
		{
			const std::string type = layer.value("type", std::string());

			const float offsetX = parentOffsetX + layer.value("offsetx", 0.0f);
			const float offsetY = parentOffsetY + layer.value("offsety", 0.0f);

			if (type == "group")
			{
				if (layer.contains("layers"))
					processLayers(layer.at("layers"), offsetX, offsetY);
				continue;
			}

			if (type != "objectgroup")
				continue;

			for (const auto& object : layer.at("objects"))
			{
				const std::string className = object.value("type", std::string());

				// Skip non-entity markers (e.g. the camera guide): no class, or not a point.
				if (className.empty() || !object.value("point", false))
					continue;

				const float x = offsetX + object.at("x").get<float>();
				const float y = offsetY + object.at("y").get<float>();

				nlohmann::json entry;
				entry["prefab"] = "data/prefabs/" + className + ".json";
				entry["Transform"]["x"] = x;
				entry["Transform"]["y"] = y;

				// patrolRange custom property -> Patrol bounds centered on the spawn
				// point. patrolDirection ("left"/"right"/"up"/"down") picks the patrol
				// axis and the initial direction (air patrollers like the blue bird);
				// without it the patrol stays horizontal, as before.
				if (object.contains("properties"))
				{
					float range = 0.0f;
					std::string direction;
					std::string facing;

					for (const auto& property : object["properties"])
					{
						const std::string name = property.value("name", std::string());

						if (name == "patrolRange")
							range = property.at("value");
						else if (name == "patrolDirection")
							direction = property.at("value");
						else if (name == "direction")
							facing = property.at("value");
					}

					if (range > 0.0f)
					{
						const bool  vertical = (direction == "up" || direction == "down");
						const float center   = vertical ? y : x;

						entry["Patrol"]["axis"] = vertical ? "Vertical" : "Horizontal";
						entry["Patrol"]["min"]  = center - range;
						entry["Patrol"]["max"]  = center + range;

						if (!direction.empty())
							entry["Patrol"]["direction"] = (direction == "left" || direction == "up") ? -1 : 1;
					}

					// "direction" ("left"/"right") mirrors a fixed-facing enemy (e.g. the
					// plant). isTextureRight stays from the prefab, so the texture is
					// flipped only when the requested side differs from its default.
					if (!facing.empty())
						entry["Facing"]["isLookingRight"] = (facing == "right");
				}

				createdEntities.push_back(LoadPrefabbedEntity(registry, entry));
			}
		}
	};

	processLayers(mapJson.at("layers"), 0.0f, 0.0f);

	AddImpliedComponents(registry, createdEntities);

	return createdEntities;
}