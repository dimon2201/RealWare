// skinning_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "skinning_data.hpp"
#include "skeleton_data.hpp"
#include "animation.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
	struct SBone;
	struct SFrame;
	class XSkinnedBonesPool;
	class XSkinPool;

	class XSkinningSubsystem :	public ISubsys
	{
		TRITON_OBJECT(XSkinningSubsystem)

		XSkinnedBonesPool* _skinnedBonesPool = nullptr;
		XSkinPool* _skinPool = nullptr;

	public:
		explicit XSkinningSubsystem(cContext* context);
		~XSkinningSubsystem() override;

		std::optional<SSkinData::THandle> Create(const SSkeletonData::THandle& skeleton);

		void Destroy(const SSkinData::THandle& skin);

		void Skin(
			const SSkinData::THandle& skin,
			const SFrame& frame
		);

		void Init() override {}

		void Free() override {}

		void Update() override;

		inline XSkinPool* GetSkinPool() const
		{
			return _skinPool;
		}

		inline XSkinnedBonesPool* GetSkinnedBonesPool() const
		{
			return _skinnedBonesPool;
		}

	private:
		void CalculateBone(
			const std::vector<SBone>& bones,
			types::usize boneIndex,
			const SFrame& frame,
			std::vector<cMatrix4>& totalTransform
		);
	};
}