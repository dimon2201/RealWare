// static_instance_storage.hpp

#pragma once

#include "storage.hpp"
#include "render_instance.hpp"
#include "handles.hpp"
#include "uploader.hpp"
#include "types.hpp"

namespace triton
{
	class cBuffer;

	struct SStaticRenderInstance
	{
		ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
		HBatch batch;
		HRenderInstance instance;
	};

	struct SGPUStaticRenderInstanceLayout final : public SRenderInstance {};

	class XStaticInstanceStorage : public IStorage<HStaticRenderInstance, SStaticRenderInstance, XLinearArray<SStaticRenderInstance>>
	{
		TRITON_OBJECT(XStaticInstanceStorage)
		TRITON_SUBSYSTEM

		cBuffer* _staticInstanceBuffer = nullptr;
		XUploader<XStaticInstanceStorage, HStaticRenderInstance, SGPUStaticRenderInstanceLayout>* _uploader = nullptr;
	
	public:
		void Update() override;
	};
}