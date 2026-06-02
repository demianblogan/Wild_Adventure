#pragma once

#include <SFML/Graphics/VertexArray.hpp>

#include <string>
#include <vector>

struct Tilemap
{
	struct Layer
	{
		std::string name;
		int width = 0;
		int height = 0;
		std::vector<int> data;
		sf::VertexArray vertices;
	};

	std::string textureName;
	int tileSize = 16;
	int atlasWidthInTiles = 0;
	int firstGid = 1;

	std::vector<Layer> layers;

	int GetWidth() const
	{
		return layers.empty() ? 0 : layers[0].width;
	}

	int GetHeight() const
	{
		return layers.empty() ? 0 : layers[0].height;
	}
};