// animation_subsystem.hpp

#pragma once

#include <string>
#include "subsystem.hpp"
#include "handles.hpp"
#include "animation.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;
	struct SBoneAnimation;

	class XAnimationSubsystem : public ISubsystem<HAnimation, SAnimation, XLinearArray<SAnimation>>
	{
		TRITON_OBJECT(XAnimationSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _skinMatrixBuffer = nullptr;

	public:
		HAnimation CreateAnimation(
			const std::string& name,
			types::f32 duration,
			types::f32 ticksPerSecond,
			HSkeleton skeleton,
			const std::vector<SBoneAnimation>& bones
		);

		void DestroyAnimation(const HAnimation& animation);

		void Evaluate(const HAnimation& animation);

		void Init() override;

		void Free() override;

		void Update() override {}

	private:
		void Upload();
	};
}