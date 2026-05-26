#include "AnimationSystem.h"

#include "components/Animation.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Sprite.h"
#include "core/ecs/Registry.h"

#include <cassert>

AnimationSystem::AnimationSystem(Registry& registry)
	: registry(registry)
{}

void AnimationSystem::Update(float deltaTime)
{
	registry.ForEach<Animation, AnimationSet, AnimationState, Sprite>(
		[deltaTime](Entity entity, Animation& animation, AnimationSet& set,
			AnimationState& state, Sprite& sprite)
		{
			if (state.current != animation.playingState)
			{
				const auto found = set.animations.find(state.current);
				assert(found != set.animations.end() && "Animation state not found in set");

				animation.data = found->second;
				animation.playingState = state.current;
				animation.currentFrame = 0;
				animation.elapsedTime = 0.0f;

				sprite.textureName = animation.data.textureName;
			}

			animation.elapsedTime += deltaTime;

			if (animation.elapsedTime < animation.data.frameDuration)
				return;

			animation.elapsedTime -= animation.data.frameDuration;
			animation.currentFrame++;

			if (animation.currentFrame >= animation.data.frameCount)
			{
				if (animation.data.isLooping)
					animation.currentFrame = 0;
				else
					animation.currentFrame = animation.data.frameCount - 1;
			}
		});
}