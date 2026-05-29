#pragma once

#include <functional>

namespace UI
{
	enum class AnimationCurve
	{
		Linear,
		Sine,
		EaseOut
	};

	enum class AnimationLoop
	{
		Once,
		Loop,
		PingPong
	};

	class Animation
	{
	public:
		Animation(float fromValue, float toValue, float duration,
			AnimationCurve curve, AnimationLoop loop,
			std::function<void(float)> setter);

		void Update(float deltaTime);

		bool IsFinished() const { return isFinished; }

		void SetOnFinished(std::function<void()> callback);

	private:
		float ApplyCurve(float t) const;

		float fromValue;
		float toValue;
		float duration;
		AnimationCurve curve;
		AnimationLoop loop;
		std::function<void(float)> setter;
		std::function<void()> onFinished;

		float elapsed = 0.0f;
		bool isFinished = false;
	};
}