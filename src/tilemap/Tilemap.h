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

	int collisionWidth = 0;
	int collisionHeight = 0;
	std::vector<bool> solidGrid;

	int GetWidth() const
	{
		return layers.empty() ? 0 : layers[0].width;
	}

	int GetHeight() const
	{
		return layers.empty() ? 0 : layers[0].height;
	}

	bool IsSolid(int tileX, int tileY) const
	{
		// Below the level is open void — the character falls through to die there.
		if (tileY >= collisionHeight)
			return false;

		// Sides and ceiling stay solid so the level is otherwise contained.
		if (tileX < 0 || tileY < 0 || tileX >= collisionWidth)
			return true;

		return solidGrid[static_cast<std::size_t>(tileY) * collisionWidth + tileX];
	}
};