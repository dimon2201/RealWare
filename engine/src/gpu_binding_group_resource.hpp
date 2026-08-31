// gpu_binding_group_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBindingGroupResource : public CGPUResource
    {
        types::qword _pool = 0;
        types::s32 _index = 0;

    public:
        explicit CGPUBindingGroupResource(
            types::qword instance,
            types::qword view,
            types::qword pool,
            types::s32 index
        ) : CGPUResource(instance, view), _pool(pool), _index(index) {}

        ~CGPUBindingGroupResource() override = default;

        static CGPUBindingGroupResource Invalid()
        {
            return CGPUBindingGroupResource(
                0, 0, 0, -1
            );
        }

        inline types::qword GetPool() const { return _pool; }

        inline types::s32 GetIndex() const { return _index; }
    };
}