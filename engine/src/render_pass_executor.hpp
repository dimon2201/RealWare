// render_pass_executor.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class XRenderPassExecutor : public iObject
    {
        TRITON_OBJECT(XRenderPassExecutor)

    public:
        explicit XRenderPassExecutor(cContext* context) : iObject(context) {}
        virtual ~XRenderPassExecutor() override = default;

        void Initialize();
        void Free();
    };
}