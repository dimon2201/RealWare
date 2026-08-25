// gpu_pipeline_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "render_pipeline_bind_point_enum.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUPipelineResource : public CGPUResource
    {
        EPipelineBindPoint _bindPoint = EPipelineBindPoint::Unknown;

    public:
        explicit CGPUPipelineResource(
            types::qword instance,
            types::qword viewInstance,
            EPipelineBindPoint bindPoint
        ) : CGPUResource(instance, viewInstance), _bindPoint(bindPoint) {}
        ~CGPUPipelineResource() override = default;

        static CGPUPipelineResource Invalid()
        {
            return CGPUPipelineResource(0, 0, EPipelineBindPoint::Unknown);
        }

        inline EPipelineBindPoint GetBindingPoint() const { return _bindPoint; }
    };
}