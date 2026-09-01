// world_render_group_instance_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "world_render_group_instance.hpp"

namespace triton
{
    class CRenderGroupInstancePool : public CObjectPool<XRenderGroupInstance>
    {
        TRITON_CLASS_NAME(CRenderGroupInstancePool)

    public:
        using CObjectPool<XRenderGroupInstance>::CObjectPool;
        ~CRenderGroupInstancePool() override = default;

        XRenderGroupInstance::TGPULayout ConvertToGpuLayout(const XRenderGroupInstance& object) override
        {
            XRenderGroupInstance::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}