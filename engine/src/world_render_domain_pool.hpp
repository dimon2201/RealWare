// world_render_domain_pool.hpp

#pragma once

#include "object_pool.hpp"
#include "world_render_domain.hpp"

namespace triton
{
    class CRenderDomainPool : public CObjectPool<XRenderDomain>
    {
        TRITON_CLASS_NAME(CRenderDomainPool)

    public:
        using CObjectPool<XRenderDomain>::CObjectPool;
        ~CRenderDomainPool() override = default;

        XRenderDomain::TGPULayout ConvertToGpuLayout(const XRenderDomain& object) override
        {
            XRenderDomain::TGPULayout gpul;

            return gpul;
        }

        void Update() override
        {
            Upload();
        }
    };
}