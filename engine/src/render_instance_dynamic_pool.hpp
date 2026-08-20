// render_instance_dynamic_pool.hpp

#pragma once

#include "render_instance_pool.hpp"

namespace triton
{
    class CRenderInstanceDynamicPool : public CRenderInstancePool
    {
        using CRenderInstancePool::CRenderInstancePool;
    };
}