// dynamic_instance_storage.hpp

#pragma once

#include "render_instance.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	class XDynamicInstanceStorage : public XUploader<SRenderInstanceData, HRenderInstance, XLinearArray<SRenderInstanceData>, SGPURenderInstanceLayout>
	{
		TRITON_OBJECT(XDynamicInstanceStorage)

	public:
		explicit XDynamicInstanceStorage(
			cContext* context,
			cGPUResource* resource,
			types::usize stagingBufferElementCount,
			types::boolean bNeedsPersistentGpuWrite
		) : XUploader(context, resource, stagingBufferElementCount, bNeedsPersistentGpuWrite) {}

		HRenderInstance CreateDynamicInstance(const HBatch& batch);

		void DestroyDynamicInstance(const HRenderInstance& instance);

		void UpdateTransform(const HRenderInstance& instance);
	};
}