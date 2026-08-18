// render_target.hpp

#pragma once

#include "object.hpp"
#include "handle.hpp"
#include "graphics_backend.hpp"

namespace triton
{
	class cContext;

	class XRenderTarget : public iObject
	{
		TRITON_OBJECT(XRenderTarget)

		CGPURenderTarget _renderTarget;

	public:
		explicit XRenderTarget(cContext* context, const CGPURenderTarget& renderTarget);
		~XRenderTarget() = default;

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}