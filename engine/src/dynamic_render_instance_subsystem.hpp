// dynamic_render_instance_subsystem.hpp

#pragma once

#include "subsystem.hpp"
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

	class XDynamicRenderInstanceSubsystem : public ISubsystem<HDynamicRenderInstance, SDynamicRenderInstance, XLinearArray<SDynamicRenderInstance>>
	{
		TRITON_OBJECT(XDynamicRenderInstanceSubsystem)
		TRITON_SUBSYSTEM

		cBuffer* _dynamicInstanceBuffer = nullptr;
		XUploader<XDynamicRenderInstanceSubsystem, HDynamicRenderInstance, SGPUDynamicRenderInstanceLayout>* _uploader = nullptr;
	};
}