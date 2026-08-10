// batch_instance.hpp

#pragma once

#include "render_instance_data.hpp"

namespace triton
{
    struct SBatchInstance
    {
        ERenderInstanceMotionType usage = ERenderInstanceMotionType::Static;
        SBatchData::THandle batch;
        SStaticRenderInstanceData::THandle staticInstance;
        SDynamicRenderInstanceData::THandle dynamicInstance;
    };
}