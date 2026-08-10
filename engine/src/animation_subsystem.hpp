// animation_subsystem.hpp

#pragma once

#include <string>
#include <optional>
#include "animation.hpp"
#include "skeleton_data.hpp"
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "types.hpp"

namespace triton
{
	struct SAnimationKey;
	class XAnimationPool;

	class XAnimationSubsystem : public ISubsys
	{
		TRITON_OBJECT(XAnimationSubsystem)

		XAnimationPool* _pool = nullptr;

	public:
		explicit XAnimationSubsystem(cContext* context);
		~XAnimationSubsystem() override;

		std::optional<SAnimationData::THandle> Create(
			const std::string& name,
			types::f32 duration,
			types::f32 ticksPerSecond,
			const SSkeletonData::THandle& skeleton,
			const std::vector<SAnimationKey>& keys
		);

		void Destroy(const SAnimationData::THandle& animation);

		std::optional<triton::SFrame> Evaluate(
			const SSkeletonData::THandle& skeleton,
			const SAnimationData::THandle& animation,
			types::f32 time
		);

		void Init() override {}

		void Free() override {}

		void Update() override {}
	};
}