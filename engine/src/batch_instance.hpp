// batch_instance.hpp

#pragma once

#include "render_instance.hpp"
#include "handles.hpp"

namespace triton
{
    struct SBatchInstance
    {
        SRenderInstance::EUsage usage = SRenderInstance::EUsage::NONE;
        HBatch batch;
        HRenderInstance instance;
    };
}