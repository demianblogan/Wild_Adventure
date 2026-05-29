#include "Animation.h"

#include <cmath>

namespace UI
{
	Animation::Animation(float fromValue, float toValue, float duration,
		AnimationCurve curve, AnimationLoop loop,
		std::function<void(float)> setter)
		: fromValue(fromValue)
		, toValue(toValue)
		, duration(duration)
		, curve(curve)
		, loop(loop)
		, setter(std::move(setter))
	{}

	void Animation::Update(float deltaTime)
	{
		if (isFinished)
			return;

		elapsed += deltaTime;

		float normalizedTime = (duration > 0.0f) ? (elapsed / duration) : 1.0f;
		float effectiveT = 0.0f;

		switch (loop)
		{
		case AnimationLoop::Once:
			if (normalizedTime >= 1.0f)
			{
				normalizedTime = 1.0f;
				isFinished = true;
			}
			effectiveT = normalizedTime;
			break;

		case AnimationLoop::Loop:
			effectiveT = normalizedTime - std::floor(normalizedTime);
			break;

		case AnimationLoop::PingPong:
		{
			const float cycle = normalizedTime - std::floor(normalizedTime / 2.0f) * 2.0f;
			effectiveT = (cycle <= 1.0f) ? cycle : (2.0f - cycle);
			break;
		}
		}

		const float curved = ApplyCurve(effectiveT);
		const float value = fromValue + (toValue - fromValue) * curved;

		if (setter)
			setter(value);

		if (isFinished && onFinished)
		{
			onFinished();
			onFinished = nullptr;
		}
	}

	void Animation::SetOnFinished(std::function<void()> callback)
	{
		onFinished = std::move(callback);
	}

	float Animation::ApplyCurve(float t) const
	{
		switch (curve)
		{
		case AnimationCurve::Linear:
			return t;

		case AnimationCurve::Sine:
			return 0.5f - 0.5f * std::cos(t * 3.14159265f);

		case AnimationCurve::EaseOut:
			return 1.0f - (1.0f - t) * (1.0f - t);
		}

		return t;
	}
}