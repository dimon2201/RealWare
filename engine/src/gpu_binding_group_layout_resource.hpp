// gpu_binding_group_layout_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBindingGroupLayoutResource : public CGPUResource
    {
    public:
        explicit CGPUBindingGroupLayoutResource(
            types::qword instance,
            types::qword view
        ) : CGPUResource(instance, view) {}

        ~CGPUBindingGroupLayoutResource() override = default;

        static CGPUBindingGroupLayoutResource Invalid()
        {
            return CGPUBindingGroupLayoutResource(0, 0);
        }
    };
}