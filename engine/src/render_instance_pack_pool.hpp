// render_instance_pack_pool.hpp

#pragma once

#include "object_pool.hpp"

namespace triton
{
    class CRenderInstancePackPool : public CObjectPool<XRenderInstancePack>
    {
        TRITON_CLASS_NAME(CRenderInstancePackPool)

    public:
        using CObjectPool<XRenderInstancePack>::CObjectPool;
        ~CRenderInstancePackPool() override = default;

        XRenderInstancePack::TGPULayout ConvertToGpuLayout(const XRenderInstancePack& object) override
        {
            XRenderInstancePack::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}