// gpu_resource.hpp

#pragma once

#include "types.hpp"

namespace triton
{
    class CGPUResource
    {
    protected:
        types::qword _instance = 0;
        types::qword _view = 0;

    public:
        explicit CGPUResource(types::qword instance, types::qword view) : _instance(instance), _view(view) {}
        
        virtual ~CGPUResource() = default;

        inline types::boolean IsValid() const
        {
            return _instance == 0 ? types::K_FALSE : types::K_TRUE;
        }

        inline void Invalidate() { _instance = 0; }

        inline types::qword GetInstance() const { return _instance; }

        inline types::qword GetView() const { return _view; }
    };
}