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
	struct SAnimationBone;
	template <typename T>
	class XUploader;

	struct SSkinnedBoneGPULayout
	{
		cMatrix4 modelToModelMatrix = cMatrix4();
	};

	class XAnimationSubsystem : public ISubsystem<HAnimation, SAnimation, XLinearArray<SAnimation>>
	{
		TRITON_OBJECT(XAnimationSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _skinnedBoneBuffer = nullptr;
		XUploader<SSkinnedBoneGPULayout>* _uploader = nullptr;

	public:
		HAnimation CreateAnimation(
			const std::string& name,
			types::f32 duration,
			types::f32 ticksPerSecond,
			HSkeleton skeleton,
			const std::vector<SAnimationBone>& bones
		);

		void DestroyAnimation(
			const HAnimation& animation
		);

		SEvaluatedFrame Evaluate(
			const HAnimation& animation,
			types::f32 time
		);

		SSkinMatrices Skin(
			const SEvaluatedFrame& frame
		);

		void Init() override;

		void Free() override;

		void Update() override;

	private:
		void Upload(cBuffer* skinMatricesBuffer);
	};
}