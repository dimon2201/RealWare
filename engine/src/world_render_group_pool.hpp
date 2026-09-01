// world_render_group_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "world_render_group.hpp"

namespace triton
{
    class CRenderGroupPool : public CObjectPool<XRenderGroup>
    {
        TRITON_CLASS_NAME(CRenderGroupPool)

    public:
        using CObjectPool<XRenderGroup>::CObjectPool;
        ~CRenderGroupPool() override = default;

        XRenderGroup::TGPULayout ConvertToGpuLayout(const XRenderGroup& object) override
        {
            XRenderGroup::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}