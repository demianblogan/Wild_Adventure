#include "DataLoader.h"

#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/PreviousTransform.h"
#include "components/Velocity.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Facing.h"
#include "components/Animation.h"
#include "core/ecs/Registry.h"

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
}

void DataLoader::AddImpliedComponents(ECS::Registry& registry, const std::vector<ECS::Entity>& entities)
{
	for (const ECS::Entity entity : entities)
	{
		if (registry.Has<ECS::Velocity>(entity) && registry.Has<ECS::Transform>(entity)
			&& !registry.Has<ECS::PreviousTransform>(entity))
		{
			const ECS::Transform& transform = registry.Get<ECS::Transform>(entity);
			registry.Add<ECS::PreviousTransform>(entity, { transform.x, transform.y });
		}

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

std::vector<ECS::Entity> DataLoader::LoadScene(ECS::Registry& registry, const std::string& scenePath)
{
	std::ifstream sceneFile(scenePath);

	if (!sceneFile.is_open())
		throw std::runtime_error("Could not open scene file: " + scenePath);

	const nlohmann::json sceneJson = nlohmann::json::parse(sceneFile);

	std::vector<ECS::Entity> createdEntities;

	for (const auto& entityJson : sceneJson.at("entities"))
	{
		nlohmann::json overrides = entityJson;

		const std::string prefabPath = overrides.at("prefab");
		overrides.erase("prefab");

		std::ifstream prefabFile(prefabPath);

		if (!prefabFile.is_open())
			throw std::runtime_error("Could not open prefab file: " + prefabPath);

		nlohmann::json merged = nlohmann::json::parse(prefabFile);
		merged.merge_patch(overrides);

		const ECS::Entity entity = LoadEntity(registry, merged);
		createdEntities.push_back(entity);
	}

	AddImpliedComponents(registry, createdEntities);

	return createdEntities;
}
