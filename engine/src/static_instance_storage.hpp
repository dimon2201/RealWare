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
	class XGameObjectSubsystem;

	class CStaticInstanceStorage : public CUploader<SRenderInstanceData, HRenderInstance, XLinearArray<SRenderInstanceData>, SGPURenderInstanceLayout>
	{
	public:
		explicit CStaticInstanceStorage(
			cContext* context,
			cGPUResource** resource,
			types::usize stagingBufferElementCount,
			types::boolean bIsPermanentDirty
		) : CUploader(context, resource, stagingBufferElementCount, bIsPermanentDirty) {}

		HRenderInstance CreateStaticInstance(const HBatch& batch);

		void DestroyStaticInstance(const HRenderInstance& instance);

		void UpdateTransform(const HRenderInstance& instance, XGameObjectSubsystem* gameObjectSubsystem);
	};
}