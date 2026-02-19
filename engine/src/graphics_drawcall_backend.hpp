// graphics_drawcall_backend.hpp

#pragma once

#include "backend.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    class iGraphicsDrawcallBackend : public iBackend
    {
        TRITON_OBJECT(iGraphicsDrawcallBackend)

    public:
        explicit iGraphicsDrawcallBackend(cContext* context);
        virtual ~iGraphicsDrawcallBackend() override = default;

        virtual void ClearColor(const cVector4& color) = 0;
        virtual void ClearDepth(types::f32 depth) = 0;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const cVector4& color) = 0;
        virtual void ClearFramebufferDepth(types::f32 depth) = 0;
        virtual void Draw(
            types::usize indexCount,
            types::usize vertexOffset,
            types::usize indexOffset,
            types::usize instanceCount
        ) = 0;
        virtual void DrawQuad() = 0;
        virtual void DrawQuads(types::usize count) = 0;
    };
}