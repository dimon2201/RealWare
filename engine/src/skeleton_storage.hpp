// skeleton_storage.hpp

#pragma once

#include "object.hpp"
#include "material.hpp"
#include "handles.hpp"
#include "math.hpp"
#include "uploader.hpp"
#include "skeleton.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	class XSkeletonStorage : public CUploader<SSkeletonData, HSkeleton, XLinearArray<SSkeletonData>, SGPUSkeletonLayout>
	{
	protected:
		explicit XSkeletonStorage(
			cContext* context,
			cGPUResource** resource,
			types::usize stagingBufferElementCount,
			types::boolean bNeedsPersistentGpuWrite
		) : CUploader(context, resource, stagingBufferElementCount, bNeedsPersistentGpuWrite) {}

		~XSkeletonStorage() = default;
	};
}