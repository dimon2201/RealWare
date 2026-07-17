// skeleton_subsystem.hpp

#pragma once

#include "handles.hpp"
#include "skeleton.hpp"
#include "subsystem.hpp"
#include "skeleton_storage.hpp"
#include "skinning_subsystem.hpp"
#include "uploader.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
	class cBuffer;

	class XSkeletonSubsystem : public ISubsys,
							   public XSkeletonStorage
	{
		TRITON_OBJECT(XSkeletonSubsystem)
		
		cBuffer* _skeletonGPUBuffer = nullptr;

	public:
		explicit XSkeletonSubsystem(cContext* context);
		~XSkeletonSubsystem() override;

		HSkeleton CreateSkeleton(
			const std::vector<SBone>& bones,
			const cMatrix4& accumulatedRootTransform = cMatrix4()
		);

		void DestroySkeleton(const HSkeleton& skeleton);

		void SetSkin(const HSkeleton& skeleton, const SSkinData& skin);

		void Init() override;

		void Free() override;

		void Update() override;

		inline cBuffer& GetSkeletonGPUBuffer() const
		{
			return *_skeletonGPUBuffer;
		}

	private:
		SGPUSkeletonLayout ConvertToGPULayout(const HSkeleton& skeleton);
	};
}