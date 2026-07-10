// graphics_context_backend.hpp

#pragma once

#include "backend.hpp"
#include "input_backend_window.hpp"

namespace triton
{
    class cContext;

    class iGraphicsContextBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsContextBackend)

    public:
        explicit iGraphicsContextBackend(cContext* context);
        virtual ~iGraphicsContextBackend() override = default;

        virtual void CreateGraphicsContext(sInputBackendWindow& window) = 0;
        virtual void MakeWindowGraphicsContextCurrent(sInputBackendWindow& window) = 0;
        virtual void SwapWindowBuffers(sInputBackendWindow& window) = 0;
    };
}