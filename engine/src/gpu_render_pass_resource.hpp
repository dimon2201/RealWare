// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPURenderPassResource : public CGPUResource
    {
        types::qword _framebuffer = 0;
        types::boolean _bClearRenderTarget = types::K_FALSE;

    public:
        explicit CGPURenderPassResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword framebuffer,
            types::boolean bClearRenderTarget
        ) : CGPUResource(instance, viewInstance), _framebuffer(framebuffer), _bClearRenderTarget(bClearRenderTarget) {}
        ~CGPURenderPassResource() override = default;

        static CGPURenderPassResource Invalid()
        {
            return CGPURenderPassResource(0, 0, 0, types::K_FALSE);
        }

        inline types::qword GetFramebuffer() const { return _framebuffer; }

        inline types::boolean IsRenderTargetClearRequired() const { return _bClearRenderTarget; }

    };
}