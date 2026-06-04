#include "Resources.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

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