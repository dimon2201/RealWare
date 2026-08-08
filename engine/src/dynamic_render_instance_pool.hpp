// dynamic_render_instance_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XDynamicRenderInstancePool : public XObjectPoolBase<SDynamicRenderInstanceData>
    {
    public:
        using XObjectPoolBase<SDynamicRenderInstanceData>::XObjectPoolBase;
        ~XDynamicRenderInstancePool() override = default;

        SDynamicRenderInstanceData::TGPULayout ConvertToGpuLayout(const SDynamicRenderInstanceData& object) override
        {
            SDynamicRenderInstanceData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}