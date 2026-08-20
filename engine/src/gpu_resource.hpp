// gpu_resource.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    class cGPUResource
    {
    protected:
        types::qword _instance = 0;
        types::qword _viewInstance = 0;

    public:
        explicit cGPUResource() = default;
        explicit cGPUResource(types::qword instance, types::qword viewInstance);
        virtual ~cGPUResource() = default;

        inline types::qword GetInstance() const { return _instance; }
        inline types::qword GetViewInstance() const { return _viewInstance; }
    };
}