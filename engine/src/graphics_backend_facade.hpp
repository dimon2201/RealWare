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
#include "types.hpp"

namespace triton
{
    struct sInputBackendWindow;
    class cApplication;
    class cTextureAtlasTexture;
    class cRenderPass;
    class cRenderTarget;
    class cTexture;
    class cShader;

    class cRenderTarget : public cGPUResource
    {
        TRITON_OBJECT(cRenderTarget)

        friend class cGraphicsOGLBackend;

        mutable std::vector<cTexture*> _colorAttachments = {};
        cTexture* _depthAttachment = nullptr;

        inline std::vector<cTexture*>& GetColorAttachments() const { return _colorAttachments; }
        inline cTexture* GetDepthAttachment() const { return _depthAttachment; }
        inline void SetDepthAttachment(cTexture* attachment) { _depthAttachment = attachment; }

    public:
        explicit cRenderTarget(cContext* context);
    };

    struct sDepthMode
    {
        types::boolean useDepthTest = types::K_TRUE;
        types::boolean useDepthWrite = types::K_TRUE;
    };

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

        // iGraphicsRenderTargetBackend
        virtual cRenderTarget* CreateRenderTarget(const std::vector<cTexture*>& colorAttachments, cTexture* depthAttachment) = 0;
        virtual void ResizeRenderTargetColors(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void ResizeRenderTargetDepth(cRenderTarget* renderTarget, const glm::vec2& size) = 0;
        virtual void UpdateRenderTargetBuffers(cRenderTarget* renderTarget) = 0;
        virtual void BindRenderTarget(const cRenderTarget* renderTarget) = 0;
        virtual void UnbindRenderTarget() = 0;
        virtual void DestroyRenderTarget(cRenderTarget* renderTarget) = 0;
        
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