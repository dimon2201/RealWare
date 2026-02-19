// graphics_context_backend.hpp

#pragma once

#include "backend.hpp"

namespace triton
{
    class cContext;

    class iGraphicsContextBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsContextBackend)

    public:
        explicit iGraphicsContextBackend(cContext* context);
        virtual ~iGraphicsContextBackend() override = default;

        virtual void CreateGraphicsContext() = 0;
    };
}