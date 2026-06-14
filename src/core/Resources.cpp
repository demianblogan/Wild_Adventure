#include "Resources.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

const nlohmann::json& Resources::GetMapJSON(const std::string& path)
{
	if (const nlohmann::json* json = TryGetJSON(path))
		return *json;

	throw std::runtime_error("Resources: cannot open map file: " + path);
}

const nlohmann::json* Resources::TryGetJSON(const std::string& path)
{
	const auto found = jsonCache.find(path);

	if (found != jsonCache.end())
		return &found->second;

	std::ifstream file(path);

	if (!file.is_open())
		return nullptr;

	return &jsonCache.emplace(path, nlohmann::json::parse(file)).first->second;
}

void Resources::LoadTexturesFromFile(const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
		throw std::runtime_error("Resources: cannot open texture manifest: " + path);

	const nlohmann::json data = nlohmann::json::parse(file);

	for (const auto& [id, texturePath] : data.items())
	{
		if (!textures.Has(id))
			textures.Load(id, texturePath.get<std::string>());
	}
}