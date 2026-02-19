// graphics_backend_facade.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "backend.hpp"
#include "math.hpp"
#include "gpu_resource.hpp"
#include "graphics_buffer_backend.hpp"
#include "graphics_pipeline_backend.hpp"
#include "types.hpp"

namespace triton
{
    struct sInputBackendWindow;
    class cApplication;
    class cTextureAtlasTexture;

    class cGraphicsBackendFacade : public iObject
    {
        TRITON_OBJECT(cGraphicsBackendFacade)

        iGraphicsBufferBackend* _buffer = nullptr;

    public:
        explicit cGraphicsBackendFacade(cContext* context, iGraphicsBufferBackend* bufferBackend);
        virtual ~cGraphicsBackendFacade() override = default;

        inline iGraphicsBufferBackend* GetBufferBackend() const { return _buffer; }

        /*virtual void BindWindowContext(void* nativeWindow) = 0;
        virtual void CreateGraphicsContext() = 0;

        // iGraphicsDrawCallBackend
        virtual void ClearColor(const glm::vec4& color) = 0;
        virtual void ClearDepth(types::f32 depth) = 0;
        virtual void ClearFramebufferColor(types::usize bufferIndex, const glm::vec4& color) = 0;
        virtual void ClearFramebufferDepth(types::f32 depth) = 0;
        virtual void Draw(types::usize indexCount, types::usize vertexOffset, types::usize indexOffset, types::usize instanceCount) = 0;
        virtual void DrawQuad() = 0;
        virtual void DrawQuads(types::usize count) = 0;*/
    };
}