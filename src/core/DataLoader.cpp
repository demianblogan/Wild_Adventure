#include "DataLoader.h"

#include "components/Sprite.h"
#include "components/Transform.h"
#include "components/Velocity.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Facing.h"
#include "core/ecs/Registry.h"


#include <fstream>
#include <stdexcept>

DataLoader::DataLoader()
{
	RegisterLoaders();
}

void DataLoader::RegisterLoaders()
{
	loaders["Transform"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			Transform transform;
			transform.x = data.at("x");
			transform.y = data.at("y");
			registry.Add<Transform>(entity, transform);
		};

	loaders["Velocity"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			Velocity velocity;
			velocity.x = data.at("x");
			velocity.y = data.at("y");
			registry.Add<Velocity>(entity, velocity);
		};

	loaders["Sprite"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			Sprite sprite;
			sprite.textureName = data.at("textureName");
			registry.Add<Sprite>(entity, sprite);
		};

	loaders["AnimationSet"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			AnimationSet set;

			for (const auto& [stateName, animationData] : data.items())
			{
				AnimationData animation;
				animation.textureName = animationData.at("textureName");
				animation.frameCount = animationData.at("frameCount");
				animation.frameDuration = animationData.at("frameDuration");
				animation.isLooping = animationData.at("isLooping");

				set.animations[stateName] = animation;
			}

			registry.Add<AnimationSet>(entity, set);
		};

	loaders["AnimationState"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			AnimationState state;
			state.current = data.at("current");
			registry.Add<AnimationState>(entity, state);
		};

	loaders["Facing"] = [](Registry& registry, Entity entity, const nlohmann::json& data)
		{
			Facing facing;
			facing.isLookingRight = data.at("isLookingRight");
			facing.isTextureRight = data.at("isTextureRight");
			registry.Add<Facing>(entity, facing);
		};
}

Entity DataLoader::LoadEntity(Registry& registry, const nlohmann::json& entityJson)
{
	const Entity entity = registry.CreateEntity();

	for (const auto& [componentName, componentData] : entityJson.items())
	{
		const auto found = loaders.find(componentName);

		if (found == loaders.end())
			throw std::runtime_error("Unknown component in data: " + componentName);

		found->second(registry, entity, componentData);
	}

	return entity;
}

Entity DataLoader::LoadEntityFromFile(Registry& registry, const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
		throw std::runtime_error("Could not open data file: " + path);

	const nlohmann::json entityJson = nlohmann::json::parse(file);

	return LoadEntity(registry, entityJson);
}
