// skeleton_subsystem.hpp

#pragma once

#include "handles.hpp"
#include "skeleton.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
	class XSkeletonSubsystem : public ISubsystem<HSkeleton, SSkeleton, XLinearArray<SSkeleton>>
	{
		TRITON_OBJECT(XSkeletonSubsystem)
		TRITON_SUBSYSTEM

	public:
		HSkeleton CreateSkeleton(const std::vector<SBone>& bones);

		void DestroySkeleton(const HSkeleton& skeleton);

		void Init() override {}

		void Free() override {}

		void Update() override {}
	};
}