// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class CGPURenderPassResource : public CGPUResource
    {
        types::qword _framebuffer = 0;
        types::boolean _bClearRenderTarget = types::K_FALSE;
        cVector4 _clearColor = cVector4(0.0f);
        types::f32 _clearDepth = 0.0f;

    public:
        explicit CGPURenderPassResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword framebuffer,
            types::boolean bClearRenderTarget,
            const cVector4& clearColor,
            types::f32 clearDepth
        ) :
            CGPUResource(instance, viewInstance),
            _framebuffer(framebuffer),
            _bClearRenderTarget(bClearRenderTarget),
            _clearColor(clearColor),
            _clearDepth(clearDepth) {}
        ~CGPURenderPassResource() override = default;

        static CGPURenderPassResource Invalid()
        {
            return CGPURenderPassResource(0, 0, 0, types::K_FALSE, cVector4(0.0f), 0.0f);
        }

        inline types::qword GetFramebuffer() const { return _framebuffer; }

        inline types::boolean IsRenderTargetClearRequired() const { return _bClearRenderTarget; }

        inline const cVector4& GetColorClearValue() const { return _clearColor; }

        inline types::f32 GetDepthClearValue() const { return _clearDepth; }
    };
}