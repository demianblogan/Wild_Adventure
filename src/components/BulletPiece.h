#pragma once

namespace ECS
{
	struct BulletPiece
	{
		int   pieceIndex   = 0;     // 0 or 1 — selects which frame of the sheet to show
		float gravityAccel = 400.0f;
	};
}
