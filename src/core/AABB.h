#pragma once

// Axis-aligned bounding box in world pixels, with the overlap test that the
// gameplay collision checks share.
struct AABB
{
	float left = 0.0f;
	float top = 0.0f;
	float right = 0.0f;
	float bottom = 0.0f;

	bool Overlaps(const AABB& other) const
	{
		return left < other.right && right > other.left
			&& top < other.bottom && bottom > other.top;
	}
};

// Most game objects anchor their Transform at the feet: x is the horizontal
// centre, y is the bottom edge. Builds the box from that convention, as used by
// Collider- and Hitbox-shaped bounds.
inline AABB FeetAABB(float centerX, float bottomY, float width, float height)
{
	const float halfWidth = width / 2.0f;
	return { centerX - halfWidth, bottomY - height, centerX + halfWidth, bottomY };
}
