// gpu_binding_group_layout_resource.hpp

#pragma once

#include "gpu_resource.hpp"
#include "render_binding_group_binding_struct.hpp"
#include "types.hpp"

namespace triton
{
    class CGPUBindingGroupLayoutResource : public CGPUResource
    {
        std::vector<SBindingGroupBinding> _bindings;

    public:
        explicit CGPUBindingGroupLayoutResource(
            types::qword instance,
            types::qword view,
            const std::vector<SBindingGroupBinding>& bindings
        ) : CGPUResource(instance, view), _bindings(bindings) {}

        ~CGPUBindingGroupLayoutResource() override = default;

        static CGPUBindingGroupLayoutResource Invalid()
        {
            return CGPUBindingGroupLayoutResource(0, 0, {});
        }

        inline const std::vector<SBindingGroupBinding>& GetBindings() const { return _bindings; }
    };
}