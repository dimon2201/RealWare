// skinning_storage.hpp

#pragma once

#include "object.hpp"
#include "material.hpp"
#include "handles.hpp"
#include "math.hpp"
#include "uploader.hpp"
#include "skeleton.hpp"
#include "skinning_data.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	class XSkinningStorage : public CUploader<SSkinningData, HSkinning, XLinearArray<SSkinningData>, SGPUSkinningLayout>
	{
	protected:
		explicit XSkinningStorage(
			cContext* context,
			cGPUResource** resource,
			types::usize stagingBufferElementCount,
			types::boolean bNeedsPersistentGpuWrite
		) : CUploader(context, resource, stagingBufferElementCount, bNeedsPersistentGpuWrite) {}

		~XSkinningStorage() = default;
	};
}