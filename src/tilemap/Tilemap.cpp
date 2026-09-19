#include "Tilemap.h"

bool Tilemap::IsDeadly(int tileX, int tileY) const
{
	if (tileX < 0 || tileY < 0 || tileX >= deathWidth || tileY >= deathHeight)
		return false;

	return deathGrid[static_cast<std::size_t>(tileY) * deathWidth + tileX];
}

bool Tilemap::IsSolid(int tileX, int tileY) const
{
	// Below the level is open void -- the character falls through to die there.
	if (tileY >= collisionHeight)
		return false;

	// Sides and ceiling stay solid so the level is otherwise contained.
	if (tileX < 0 || tileY < 0 || tileX >= collisionWidth)
		return true;

	return solidGrid[static_cast<std::size_t>(tileY) * collisionWidth + tileX];
}
