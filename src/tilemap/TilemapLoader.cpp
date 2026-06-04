#include "TilemapLoader.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <stdexcept>

static void BuildLayerVertices(Tilemap::Layer& layer, int tileSize, int atlasWidthInTiles, int firstGid)
{
	layer.vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
	layer.vertices.clear();

	for (int y = 0; y < layer.height; y++)
	{
		for (int x = 0; x < layer.width; x++)
		{
			const int index = y * layer.width + x;
			const int gid = layer.data[index];

			if (gid == 0)
				continue;

			const int tileIndex = gid - firstGid;
			const int atlasX = tileIndex % atlasWidthInTiles;
			const int atlasY = tileIndex / atlasWidthInTiles;

			const float worldX = static_cast<float>(x * tileSize);
			const float worldY = static_cast<float>(y * tileSize);
			const float texX = static_cast<float>(atlasX * tileSize);
			const float texY = static_cast<float>(atlasY * tileSize);
			const float size = static_cast<float>(tileSize);

			const sf::Vector2f topLeft = { worldX,        worldY };
			const sf::Vector2f topRight = { worldX + size, worldY };
			const sf::Vector2f bottomRight = { worldX + size, worldY + size };
			const sf::Vector2f bottomLeft = { worldX,        worldY + size };

			const sf::Vector2f texTopLeft = { texX,        texY };
			const sf::Vector2f texTopRight = { texX + size, texY };
			const sf::Vector2f texBottomRight = { texX + size, texY + size };
			const sf::Vector2f texBottomLeft = { texX,        texY + size };

			// First triangle: top-left, top-right, bottom-right
			layer.vertices.append({ topLeft,     sf::Color::White, texTopLeft });
			layer.vertices.append({ topRight,    sf::Color::White, texTopRight });
			layer.vertices.append({ bottomRight, sf::Color::White, texBottomRight });

			// Second triangle: top-left, bottom-right, bottom-left
			layer.vertices.append({ topLeft,     sf::Color::White, texTopLeft });
			layer.vertices.append({ bottomRight, sf::Color::White, texBottomRight });
			layer.vertices.append({ bottomLeft,  sf::Color::White, texBottomLeft });
		}
	}
}

Tilemap LoadTilemap(const std::string& path, const std::string& textureName, int atlasWidthInTiles)
{
	std::ifstream file(path);
	if (!file)
		throw std::runtime_error("LoadTilemap: cannot open file '" + path + "'");

	nlohmann::json data;
	file >> data;

	Tilemap tilemap;
	tilemap.textureName = textureName;
	tilemap.atlasWidthInTiles = atlasWidthInTiles;
	tilemap.tileSize = data.value("tilewidth", 16);

	if (data.contains("tilesets") && !data["tilesets"].empty())
		tilemap.firstGid = data["tilesets"][0].value("firstgid", 1);

	if (data.contains("layers"))
	{
		for (const auto& layerData : data["layers"])
		{
			const std::string type = layerData.value("type", "");
			if (type != "tilelayer")
				continue;

			Tilemap::Layer layer;
			layer.name = layerData.value("name", "");
			layer.width = layerData.value("width", 0);
			layer.height = layerData.value("height", 0);

			if (layerData.contains("data"))
			{
				for (const auto& gid : layerData["data"])
					layer.data.push_back(gid);
			}

			// The "collision" layer is physics-only: build the solid grid, don't draw it.
			if (layer.name == "collision")
			{
				tilemap.collisionWidth = layer.width;
				tilemap.collisionHeight = layer.height;
				tilemap.solidGrid.assign(static_cast<std::size_t>(layer.width) * layer.height, false);

				for (std::size_t i = 0; i < layer.data.size(); i++)
					tilemap.solidGrid[i] = (layer.data[i] != 0);

				continue;
			}

			BuildLayerVertices(layer, tilemap.tileSize, tilemap.atlasWidthInTiles, tilemap.firstGid);

			tilemap.layers.push_back(std::move(layer));
		}
	}

	return tilemap;
}