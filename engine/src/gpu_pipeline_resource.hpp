// gpu_pipeline_resource.hpp

#pragma once

#include <vector>
#include "gpu_resource.hpp"
#include "render_pipeline_bind_point_enum.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUPipelineResource : public CGPUResource
    {
        EPipelineBindPoint _bindPoint = EPipelineBindPoint::Unknown;
        std::vector<types::qword> _descriptorSets;

    public:
        explicit CGPUPipelineResource(
            types::qword instance,
            types::qword viewInstance,
            EPipelineBindPoint bindPoint,
            const std::vector<types::qword>& descriptorSets
        ) :
            CGPUResource(instance, viewInstance),
            _bindPoint(bindPoint),
            _descriptorSets(descriptorSets) {}
        ~CGPUPipelineResource() override = default;

        static CGPUPipelineResource Invalid()
        {
            return CGPUPipelineResource(0, 0, EPipelineBindPoint::Unknown, {});
        }

        inline EPipelineBindPoint GetBindingPoint() const { return _bindPoint; }

        inline const std::vector<types::qword> GetDescriptorSets() const { return _descriptorSets; }
    };
}