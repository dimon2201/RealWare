// batch_instance.hpp

#pragma once

#include "render_instance.hpp"
#include "handles.hpp"

namespace triton
{
    struct SBatchInstance
    {
        ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
        HBatch batch;
        HRenderInstance instance;
    };
}