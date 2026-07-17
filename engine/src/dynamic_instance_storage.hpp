// dynamic_instance_storage.hpp

#pragma once

#include "render_instance.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;
	class XGameObjectSubsystem;

	class CDynamicInstanceStorage : public CUploader<SRenderInstanceData, HRenderInstance, XLinearArray<SRenderInstanceData>, SGPURenderInstanceLayout>
	{
	public:
		explicit CDynamicInstanceStorage(
			cContext* context,
			cGPUResource** resource,
			types::usize stagingBufferElementCount,
			types::boolean bNeedsPersistentGpuWrite
		) : CUploader(context, resource, stagingBufferElementCount, bNeedsPersistentGpuWrite) {}

		HRenderInstance CreateDynamicInstance(const HBatch& batch);

		void DestroyDynamicInstance(const HRenderInstance& instance);

		void UpdateTransform(const HRenderInstance& instance, XGameObjectSubsystem* gameObjectSubsystem);
	};
}