// skeleton_subsystem.hpp

#pragma once

#include <optional>
#include "skeleton_data.hpp"
#include "skinning_subsystem.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
	class cBuffer;
	class XSkeletonPool;

	class XSkeletonSubsystem : public ISubsys
	{
		TRITON_OBJECT(XSkeletonSubsystem)

		XSkeletonPool* _pool = nullptr;

	public:
		explicit XSkeletonSubsystem(cContext* context);
		~XSkeletonSubsystem() override;

		std::optional<SSkeletonData::THandle> Create(
			const std::vector<SBone>& bones,
			const cMatrix4& accumulatedRootTransform = cMatrix4()
		);

		void Destroy(const SSkeletonData::THandle& skeleton);

		void Init() override;

		void Free() override;

		void Update() override;

		inline XSkeletonPool* GetPool() const
		{
			return _pool;
		}
	};
}