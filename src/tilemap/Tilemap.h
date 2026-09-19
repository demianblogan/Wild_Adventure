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

	int deathWidth = 0;
	int deathHeight = 0;
	std::vector<bool> deathGrid;

	int GetWidth() const { return layers.empty() ? 0 : layers[0].width; }
	int GetHeight() const { return layers.empty() ? 0 : layers[0].height; }

	// Touching a "death" tile kills the hero as if he fell into a pit, so pits on
	// tall maps don't need a long fall to the world's bottom edge.
	bool IsDeadly(int tileX, int tileY) const;

	bool IsSolid(int tileX, int tileY) const;
};