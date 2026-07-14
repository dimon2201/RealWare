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
	struct SBone;
	struct SFrame;

	struct SGPUSkinnedBoneLayout
	{
		cMatrix4 modelMatrix = cMatrix4();
	};

	struct SSkinData
	{
		types::usize globSkinnedBoneOffset = 0;
		std::vector<HSkinnedBone> skinnedBones = {};
	};

	class XSkinningSubsystem : public ISubsystem<HSkinnedBone, SSkinnedBoneData, XLinearArray<SSkinnedBoneData>>
	{
		TRITON_OBJECT(XSkinningSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _skinnedBoneBuffer = nullptr;
		XUploader<XSkinningSubsystem, HSkinnedBone, SGPUSkinnedBoneLayout>* _uploader = nullptr;

	public:
		// TODO: create separate structure for skin instead of std::vector<HSkinnedBone>
		SSkinData CreateSkin(
			const HSkeleton& skeleton,
			const SFrame& frame
		);

		void DestroySkin(const SSkinData& skin);

		void Init() override;

		void Free() override;

		void Update() override;

		inline cBuffer* GetSkinnedBoneBuffer()
		{
			return _skinnedBoneBuffer;
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