#include "AnimationSystem.h"

#include "components/Animation.h"
#include "components/AnimationSet.h"
#include "components/AnimationState.h"
#include "components/Sprite.h"
#include "components/Transform.h"
#include "core/ecs/Registry.h"
#include "graphics/ParticleSystem.h"

#include <cassert>

namespace ECS
{
	AnimationSystem::AnimationSystem(Registry& registry, ParticleSystem* particles)
		: registry(registry)
		, particles(particles)
	{}

	void AnimationSystem::Update(float deltaTime)
	{
		registry.ForEach<Animation, AnimationSet, AnimationState, Sprite>(
			[this, deltaTime](Entity entity, Animation& animation, AnimationSet& set,
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
					animation.isFinished = false;

					sprite.textureName = animation.data.textureName;
				}

				animation.elapsedTime += deltaTime;

				if (animation.elapsedTime < animation.data.frameDuration)
					return;

				animation.elapsedTime -= animation.data.frameDuration;

				const int endedFrame = animation.currentFrame;
				animation.currentFrame++;

				if (animation.currentFrame >= animation.data.frameCount)
				{
					if (animation.data.isLooping)
						animation.currentFrame = 0;
					else
					{
						animation.currentFrame = animation.data.frameCount - 1;
						animation.isFinished = true;
					}
				}

				if (particles != nullptr
					&& endedFrame == animation.data.particleFrame
					&& !animation.data.particlePreset.empty()
					&& registry.Has<Transform>(entity))
				{
					const Transform& transform = registry.Get<Transform>(entity);
					particles->Emit(animation.data.particlePreset, { transform.x, transform.y });
				}
			});
	}
}
