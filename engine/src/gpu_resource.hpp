// gpu_resource.hpp

#pragma once

#include "object.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class cGPUResource : public iObject
    {
        TRITON_OBJECT(cGPUResource)

    protected:
        types::qword _instance = 0;
        types::qword _viewInstance = 0;

    public:
        explicit cGPUResource(cContext* context, types::qword instance, types::qword viewInstance);
        virtual ~cGPUResource() override = default;

        inline types::qword GetInstance() const { return _instance; }
        inline types::qword GetViewInstance() const { return _viewInstance; }
    };
}