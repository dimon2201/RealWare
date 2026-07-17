// skinning_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include "subsystem.hpp"
#include "handles.hpp"
#include "skinning_data.hpp"
#include "animation.hpp"
#include "uploader.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
	class cBuffer;
	struct SBone;
	struct SFrame;

	// TODO: rethink this or move this to separate file
	// ||||||||||||||||||||||||||||||||||||||||||||||||
	// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	struct SSkinData
	{
		types::usize globSkinnedBoneBufferOffset = 0;
		std::vector<HSkinning> skinnedBones = {};
	};
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	// ||||||||||||||||||||||||||||||||||||||||||||||||

	class XSkinningSubsystem :	public ISubsys,
								public CUploader<SSkinningData, HSkinning, XLinearArray<SSkinningData>, SGPUSkinningLayout>
	{
		TRITON_OBJECT(XSkinningSubsystem)

		cBuffer* _skinningGPUBuffer = nullptr;

	public:
		explicit XSkinningSubsystem(cContext* context);
		~XSkinningSubsystem() override;

		// TODO: create separate structure for skin instead of std::vector<HSkinnedBone>
		SSkinData CreateSkin(
			const HSkeleton& skeleton,
			const SFrame& frame
		);

		void DestroySkin(const SSkinData& skin);

		void Init() override;

		void Free() override;

		void Update() override;

		inline cBuffer& GetSkinningGPUBuffer()
		{
			return *_skinningGPUBuffer;
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