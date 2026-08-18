// render_target.cpp

#include "render_target.hpp"

triton::XRenderTarget::XRenderTarget(
	cContext* context,
	const CGPURenderTarget& renderTarget
) : iObject(context), _renderTarget(renderTarget) {}