// static_render_instance_pool.hpp

#pragma once

#include "object_pool_base.hpp"

namespace triton
{
    class XStaticRenderInstancePool : public XObjectPoolBase<SStaticRenderInstanceData>
    {
    public:
        using XObjectPoolBase<SStaticRenderInstanceData>::XObjectPoolBase;
        ~XStaticRenderInstancePool() override = default;

        SStaticRenderInstanceData::TGPULayout ConvertToGpuLayout(const SStaticRenderInstanceData& object) override
        {
            SStaticRenderInstanceData::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}