// render_target.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "handle.hpp"
#include "gpu_render_target_resource.hpp"
#include "texture.hpp"

namespace triton
{
	class cContext;

	class XRenderTarget : public iObject
	{
		TRITON_OBJECT(XRenderTarget)

		std::vector<XTexture::THandle> _colorAttachments;
		XTexture::THandle _depthAttachment;
		CGPURenderTargetResource _gpuRenderTarget = CGPURenderTargetResource::Invalid();

	public:
		explicit XRenderTarget(
			cContext* context,
			types::s32 poolIndex,
			const std::vector<XTexture::THandle>& colorAttachments,
			const XTexture::THandle& depthAttachment
		);

		~XRenderTarget() override;

		inline const CGPURenderTargetResource& GetGPUResource() const { return _gpuRenderTarget; }

		inline const std::vector<XTexture::THandle>& GetColorAttachments() const { return _colorAttachments; }

		inline const XTexture::THandle& GetDepthAttachment() const { return _depthAttachment; }

		inline void SetColorAttachmentLayout(
			types::usize attachmentIndex,
			EGraphicsImageLayout srcLayout,
			EGraphicsImageLayout dstLayout
		)
		{
			_gpuRenderTarget.SetColorAttachmentLayout(attachmentIndex, srcLayout, dstLayout);
		}

		inline void SetDepthAttachmentLayout(
			EGraphicsImageLayout srcLayout,
			EGraphicsImageLayout dstLayout
		)
		{
			_gpuRenderTarget.SetDepthAttachmentLayout(srcLayout, dstLayout);
		}

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}