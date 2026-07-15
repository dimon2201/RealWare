// static_render_instance_subsystem.hpp

#pragma once

#include "subsystem.hpp"
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

	class XStaticRenderInstanceSubsystem : public ISubsystem<HStaticRenderInstance, SStaticRenderInstance, XLinearArray<SStaticRenderInstance>>
	{
		TRITON_OBJECT(XStaticRenderInstanceSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _staticInstanceBuffer = nullptr;
		XUploader<XStaticRenderInstanceSubsystem, HStaticRenderInstance, SGPUStaticRenderInstanceLayout>* _uploader = nullptr;
	};
}