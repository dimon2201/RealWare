#include "render_pass.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "graphics_pipeline_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XRenderPass::XRenderPass(cContext* context) : iObject(context)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();

    std::vector<cShader::sDefinePair> definePairs = {};
    cVertexArray* vertexArray = nullptr;
    cShader* shader = nullptr;

    vertexArray = CreateVertexArray();
    BindVertexArray(vertexArray);
    if (desc._inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_NONE)
    {
        for (auto buffer : desc._inputBuffers)
            gfxResourceBackend->BindBuffer(buffer);
    }
    else if (desc._inputVertexFormat == eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3)
    {
        for (auto buffer : desc._inputBuffers)
            gfxResourceBackend->BindBuffer(buffer);

        BindDefaultInputLayout();
    }
    UnbindVertexArray();

    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    XInstanceBuffer* instanceBufferStatic = _context->Create<XInstanceBuffer>(
        _context,
        gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderStaticInstanceCount * sizeof(SRenderInstance), 0)
    );
    XInstanceBuffer* instanceBufferDynamic = _context->Create<XInstanceBuffer>(
        _context,
        gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderDynamicInstanceCount * sizeof(SRenderInstance), 0)
    );
    cBuffer* materialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    cBuffer* textureBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);

    return _context->Create<XRenderPassGPU>(_context, vertexArray, shader, instanceBufferStatic, instanceBufferDynamic, materialBuffer, textureBuffer);

    const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
    sChunkAllocatorDescriptor cad = {};
    cad.chunkByteSize = caps->hashTableChunkByteSize;
    cad.maxChunkCount = caps->hashTableMaxChunkCount;
    cad.hashTableSize = caps->hashTableSize;
    _dirtyStaticInstances = _context->Create<cStack<SInstanceBufferHandle>>(_context, cad);
}

void triton::XRenderPass::WriteDirtyStaticInstancesToGPU()
{
    while (_dirtyStaticInstances->IsEmpty())
    {
        SInstanceBufferHandle handle = _dirtyStaticInstances->Pop();
    }
}

void triton::XRenderPass::WriteDynamicInstancesToGPU()
{
}

void triton::XRenderPass::WriteStaticInstanceToGPU(const SInstanceBufferHandle& instance)
{
    _dirtyStaticInstances->Push(std::move(instance));
}

void triton::XRenderPass::SynchronizeGPU()
{
}

void triton::XRenderPass::Execute()
{
}

void triton::XRenderPass::ResizeViewport(const cVector2& size)
{
    _desc._viewport.rect.SetZ(size.GetX());
    _desc._viewport.rect.SetW(size.GetY());
}

void triton::XRenderPass::ResizeColorAttachments(const cVector2& size)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->ResizeRenderTargetColors(GetRenderTarget(), glm::vec2(size.GetX(), size.GetY()));
}

void triton::XRenderPass::ResizeDepthAttachment(const cVector2& size)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    cRenderTarget* renderTarget = GetRenderTarget();
    renderTarget->SetDepthAttachment(
        gfxResourceBackend->ResizeTexture(
            renderTarget->GetDepthAttachment(),
            size
        )
    );
}

const std::vector<triton::SShaderDefine>&& SetInputTextures(const std::vector<triton::SRenderPassTexture>& textures)
{
    std::vector<triton::SShaderDefine> defines = {};
    for (usize i = 0; i < textures.size(); i++)
    {
        const usize textureAtlasTextureIndex = i;
        const std::string& textureAtlasTextureName = textures[i]._name;
        defines.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
    }

    return std::move(defines);
}

void triton::XRenderPass::SetShader(const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFunc, const std::string& fragmentCustomFunc, const std::vector<SShaderDefine>&& defines)
{
    if (!_shader)
        _context->Destroy<XShader>(_shader);
    
    _shader = _context->Create<XShader>(_context, vertexStr, fragmentStr, vertexCustomFunc, fragmentCustomFunc, defines);
}

triton::cVertexArray* triton::XRenderPass::GetVertexArray() const
{
    return _renderPassGPU->GetVertexArray();
}

triton::cShader* triton::XRenderPass::GetShader() const
{
    return _renderPassGPU->GetShader();
}

triton::XInstanceBuffer* triton::XRenderPass::GetInstanceBuffer() const
{
    return _renderPassGPU->GetInstanceBuffer();
}

triton::cBuffer* triton::XRenderPass::GetMaterialBuffer() const
{
    return _renderPassGPU->GetMaterialBuffer();
}

triton::cBuffer* triton::XRenderPass::GetTextureBuffer() const
{
    return _renderPassGPU->GetTextureBuffer();
}