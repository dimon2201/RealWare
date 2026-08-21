// render_instance_dynamic_pool.hpp

#pragma once

#include "render_instance_pool.hpp"

namespace triton
{
    class CRenderInstanceDynamicPool : public CRenderInstancePool
    {
        TRITON_CLASS_NAME(CRenderInstanceDynamicPool)

        using CRenderInstancePool::CRenderInstancePool;
    };
}