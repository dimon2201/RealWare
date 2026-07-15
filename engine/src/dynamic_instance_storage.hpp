// dynamic_instance_storage.hpp

#pragma once

#include "storage.hpp"
#include "render_instance.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	struct SDynamicRenderInstance
	{
		ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
		HBatch batch;
		HRenderInstance instance;
	};

	struct SGPUDynamicRenderInstanceLayout final : public SRenderInstance {};

	class XDynamicInstanceStorage : public IStorage<HDynamicRenderInstance, SDynamicRenderInstance, XLinearArray<SDynamicRenderInstance>>
	{
		TRITON_OBJECT(XDynamicInstanceStorage)
		TRITON_STORAGE

		cBuffer* _dynamicInstanceBuffer = nullptr;
		XUploader<XDynamicInstanceStorage, HDynamicRenderInstance, SGPUDynamicRenderInstanceLayout>* _uploader = nullptr;
	
	public:
		void Update() override;
	};
}