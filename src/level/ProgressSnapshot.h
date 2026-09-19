#pragma once

#include <SFML/System/Vector2.hpp>

#include <vector>

// Snapshot of which collectibles/boxes/enemies were still alive at the last
// checkpoint, so a death can restore the world to that exact state instead of
// respawning into a level that still looks freshly started.
struct ProgressSnapshot
{
	std::vector<sf::Vector2f> aliveCollectibles; // positions of uncollected fruits at checkpoint
	std::vector<sf::Vector2f> aliveBoxes;        // positions of unbroken boxes at checkpoint
	std::vector<sf::Vector2f> aliveEnemies;      // spawn positions of living enemies at checkpoint
};
