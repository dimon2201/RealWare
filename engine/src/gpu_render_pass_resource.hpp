// gpu_render_pass_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPURenderPassResource : public CGPUResource
    {
        types::qword _framebuffer = 0;

    public:
        explicit CGPURenderPassResource(
            types::qword instance,
            types::qword viewInstance,
            types::qword framebuffer
        ) : CGPUResource(instance, viewInstance), _framebuffer(framebuffer) {}
        ~CGPURenderPassResource() override = default;

        static CGPURenderPassResource Invalid()
        {
            return CGPURenderPassResource(0, 0, 0);
        }

        inline types::qword GetFramebuffer() const { return _framebuffer; }
    };
}