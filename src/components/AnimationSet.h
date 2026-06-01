#pragma once

#include "AnimationData.h"

#include <string>
#include <unordered_map>

namespace ECS
{
	struct AnimationSet
	{
		std::unordered_map<std::string, AnimationData> animations;
	};
}