// skeleton_subsystem.hpp

#pragma once

#include "handles.hpp"
#include "skeleton.hpp"
#include "subsystem.hpp"
#include "skinning_subsystem.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	struct SGPUSkeletonLayout
	{
		types::u32 globSkinnedBoneOffset = 0;
	};

	class XSkeletonSubsystem : public ISubsystem<HSkeleton, SSkeleton, XLinearArray<SSkeleton>>
	{
		TRITON_OBJECT(XSkeletonSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _skeletonBuffer = nullptr;
		CUploader<SSkeleton, HSkeleton, XLinearArray<SSkeleton>, SGPUSkeletonLayout>* _uploader = nullptr;

	public:
		HSkeleton CreateSkeleton(
			const std::vector<SBone>& bones,
			types::usize globSkinnedBoneOffset = 0,
			const cMatrix4& accumulatedRootTransform = cMatrix4()
		);

		void DestroySkeleton(const HSkeleton& skeleton);

		void SetSkin(const HSkeleton& skeleton, const SSkinData& skin);

		void Init() override;

		void Free() override;

		void Update() override;

		inline cBuffer* GetSkeletonBuffer()
		{
			return _skeletonBuffer;
		}
	};
}