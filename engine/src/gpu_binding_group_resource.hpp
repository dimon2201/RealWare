// gpu_binding_group_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBindingGroupResource : public CGPUResource
    {
        types::qword _pool = 0;

    public:
        explicit CGPUBindingGroupResource(
            types::qword instance,
            types::qword view,
            types::qword pool
        ) : CGPUResource(instance, view), _pool(pool) {}

        ~CGPUBindingGroupResource() override = default;

        static CGPUBindingGroupResource Invalid()
        {
            return CGPUBindingGroupResource(
                0, 0, 0
            );
        }

        inline types::qword GetPool() const { return _pool; }
    };
}