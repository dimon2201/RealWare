// graphics_context_backend_ogl.hpp

#pragma once

#include "graphics_context_backend.hpp"

namespace triton
{
    class cContext;

    class cGraphicsContextBackendOGL final : public iGraphicsContextBackend
    {
        TRITON_OBJECT(cGraphicsContextBackendOGL)

    public:
        explicit cGraphicsContextBackendOGL(cContext* context);
        virtual ~cGraphicsContextBackendOGL() override final = default;

        virtual void CreateGraphicsContext() override final;
    };
}