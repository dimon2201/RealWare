// static_instance_storage.hpp

#pragma once

#include "object.hpp"
#include "render_instance.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	class XStaticInstanceStorage : public XUploader<SRenderInstanceData, HRenderInstance, XLinearArray<SRenderInstanceData>, SGPURenderInstanceLayout>
	{
		TRITON_OBJECT(XStaticInstanceStorage)

	public:
		explicit XStaticInstanceStorage(
			cContext* context,
			cGPUResource* resource,
			types::usize stagingBufferElementCount,
			types::boolean bIsPermanentDirty
		) : XUploader(context, resource, stagingBufferElementCount, bIsPermanentDirty) {}

		HRenderInstance CreateStaticInstance(const HBatch& batch);

		void DestroyStaticInstance(const HRenderInstance& instance);

		void UpdateTransform(const HRenderInstance& instance);
	};
}