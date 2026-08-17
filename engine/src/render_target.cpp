// render_target.cpp

#include "render_target.hpp"

triton::XRenderTarget::XRenderTarget(
	cContext* context,
	XRenderTargetBackend* renderTarget
) : iObject(context), _renderTarget(renderTarget) {}