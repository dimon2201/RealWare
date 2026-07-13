// skinning_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include "subsystem.hpp"
#include "handles.hpp"
#include "skinned_bone_data.hpp"
#include "animation.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;
	template <typename HSkinnedBone, typename XSkinningSubsystem, typename SSkinnedBoneGPULayout>
	class XUploader;

	struct SSkinnedBoneGPULayout
	{
		cMatrix4 modelMatrix = cMatrix4();
	};

	class XSkinningSubsystem : public ISubsystem<HSkinnedBone, SSkinnedBoneData, XLinearArray<SSkinnedBoneData>>
	{
		TRITON_OBJECT(XSkinningSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _skinnedBoneBuffer = nullptr;
		XUploader<HSkinnedBone, XSkinningSubsystem, SSkinnedBoneGPULayout>* _uploader = nullptr;

	public:
		// TODO: create separate structure for skin instead of std::vector<HSkinnedBone>
		std::vector<HSkinnedBone> CreateSkin(const SEvaluatedFrame& frame);

		void DestroySkin(const std::vector<HSkinnedBone>& animation);

		void Init() override;

		void Free() override;

		void Update() override;
	};
}