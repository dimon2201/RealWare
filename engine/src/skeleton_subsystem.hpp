// skeleton_subsystem.hpp

#pragma once

#include "handles.hpp"
#include "skeleton.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;
	template <typename TSubsystem, typename TCPUObjectHandle, typename TGPUElementLayout>
	class XUploader;

	struct SGPUSkeletonLayout
	{
		types::u32 globBoneOffset = 0;
	};

	class XSkeletonSubsystem : public ISubsystem<HSkeleton, SSkeleton, XLinearArray<SSkeleton>>
	{
		TRITON_OBJECT(XSkeletonSubsystem)
		TRITON_SUBSYSTEM

		types::usize _totalBoneCount = 0;
		cBuffer* _skeletonBuffer = nullptr;
		XUploader<XSkeletonSubsystem, HSkeleton, SGPUSkeletonLayout>* _uploader = nullptr;

	public:
		HSkeleton CreateSkeleton(const std::vector<SBone>& bones);

		void DestroySkeleton(const HSkeleton& skeleton);

		void Init() override;

		void Free() override;

		void Update() override;
	};
}