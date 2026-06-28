// graphics_drawcall_backend_ogl.hpp

#pragma once

#include "graphics_drawcall_backend.hpp"

namespace triton
{
    class cContext;

    class cGraphicsDrawcallBackendOGL final : public iGraphicsDrawcallBackend
    {
        TRITON_OBJECT(cGraphicsDrawcallBackendOGL)

    public:
        explicit cGraphicsDrawcallBackendOGL(cContext* context);
        virtual ~cGraphicsDrawcallBackendOGL() override final = default;

        virtual void ClearColor(const cVector4& color) override final;
        virtual void ClearDepth(types::f32 depth) override final;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) override final;
        virtual void ClearFramebufferDepth(types::f32 depth) override final;
        virtual void Draw(
            types::usize indexCount,
            types::usize vertexElementOffset,
            types::usize indexElementOffset,
            types::usize instanceCount
        ) override final;
        virtual void DrawQuad() override final;
        virtual void DrawQuads(types::usize count) override final;
    };
}