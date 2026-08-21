// render_instance_static_pool.hpp

#pragma once

#include "render_instance_pool.hpp"

namespace triton
{
    class CRenderInstanceStaticPool : public CRenderInstancePool
    {
        TRITON_CLASS_NAME(CRenderInstanceStaticPool)

        using CRenderInstancePool::CRenderInstancePool;
    };
}