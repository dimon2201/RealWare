// render_pass.hpp

#pragma once

#include "object.hpp"
#include "render_pass_dispatch_enum.hpp"
#include "gpu_render_pass_resource.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

	struct IRenderPass : public iObject
	{
        TRITON_CLASS_NAME(IRenderPass)

    protected:
        CGPURenderPassResource _gpuRenderPass = CGPURenderPassResource::Invalid();
        ERenderPassDispatch _dispatch = ERenderPassDispatch::None;
        
    public:
        explicit IRenderPass(
            cContext* context,
            types::s32 poolIndex,
            ERenderPassDispatch dispatch
        ) : iObject(context, poolIndex), _dispatch(dispatch) {}
        ~IRenderPass() override = default;

        virtual void Render() = 0;
	};
}