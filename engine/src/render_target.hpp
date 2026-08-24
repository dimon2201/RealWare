// render_target.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "handle.hpp"
#include "gpu_render_target_resource.hpp"

namespace triton
{
	class cContext;

	class XRenderTarget : public iObject
	{
		TRITON_OBJECT(XRenderTarget)

		CGPURenderTargetResource _gpuRenderTarget = CGPURenderTargetResource::Invalid();

	public:
		explicit XRenderTarget(
			cContext* context,
			types::s32 poolIndex,
			const std::vector<CGPUTextureResource>& colorAttachments,
			const CGPUTextureResource& depthAttachment
		);

		~XRenderTarget() override;

		inline const CGPURenderTargetResource& GetGPUResource() const { return _gpuRenderTarget; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}