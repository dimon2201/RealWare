// render_target.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"

namespace triton
{
	class cContext;
	class cTexture;
	class XRenderTargetBackend;

	class XRenderTarget : public iObject
	{
		TRITON_OBJECT(XRenderTarget)

		XRenderTargetBackend* _renderTarget = nullptr;

	public:
		explicit XRenderTarget(cContext* context, XRenderTargetBackend* renderTarget);
		~XRenderTarget() = default;

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}