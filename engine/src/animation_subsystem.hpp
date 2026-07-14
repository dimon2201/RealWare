// animation_subsystem.hpp

#pragma once

#include <string>
#include "subsystem.hpp"
#include "handles.hpp"
#include "animation.hpp"
#include "types.hpp"

namespace triton
{
	struct SAnimationKey;

	class XAnimationSubsystem : public ISubsystem<HAnimation, SAnimation, XLinearArray<SAnimation>>
	{
		TRITON_OBJECT(XAnimationSubsystem)
		TRITON_SUBSYSTEM

	public:
		HAnimation CreateAnimation(
			const std::string& name,
			types::f32 duration,
			types::f32 ticksPerSecond,
			HSkeleton skeleton,
			const std::vector<SAnimationKey>& bones
		);

		void DestroyAnimation(const HAnimation& animation);

		SFrame Evaluate(
			const HSkeleton& skeleton,
			const HAnimation& animation,
			types::f32 time
		);

		void Init() override {}

		void Free() override {}

		void Update() override {}
	};
}