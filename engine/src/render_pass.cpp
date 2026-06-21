#include "render_pass.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "graphics_resource_backend.hpp"
#include "graphics_pipeline_backend.hpp"
#include "instance_buffer.hpp"
#include "components.hpp"
#include "stack.hpp"
#include "application.hpp"

using namespace triton::ecs::components;
using namespace types;

triton::XRenderPass::XRenderPass(cContext* context) : iObject(context)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    _instanceBufferStatic = _context->Create<XInstanceBuffer>(
        _context,
        gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderStaticInstanceCount * sizeof(SRenderInstance), 0)
    );
    _instanceBufferDynamic = _context->Create<XInstanceBuffer>(
        _context,
        gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderDynamicInstanceCount * sizeof(SRenderInstance), 0)
    );
    _materialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    _textureBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    
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
        //SInstanceBufferHandle handle = _dirtyStaticInstances->Pop();
        //_instanceBufferStatic->Write(handle);
    }
}

void triton::XRenderPass::WriteDynamicInstancesToGPU()
{
    _instanceBufferDynamic->WriteAll();
}

void triton::XRenderPass::WriteStaticInstanceToGPU(const SInstanceBufferHandle& instance)
{
    _dirtyStaticInstances->Push(std::move(instance));
}

void triton::XRenderPass::SynchronizeWithGPU()
{
    WriteDirtyStaticInstancesToGPU();
    WriteDynamicInstancesToGPU();
}

void triton::XRenderPass::Execute()
{
}

void triton::XRenderPass::ResizeViewport(const cVector2& size)
{
    _viewport.rect.SetZ(size.GetX());
    _viewport.rect.SetW(size.GetY());
}

void triton::XRenderPass::ResizeColorAttachments(const cVector2& size)
{
    iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
    gfxPipelineBackend->ResizeRenderTargetColors(_renderTarget, glm::vec2(size.GetX(), size.GetY()));
}

void triton::XRenderPass::ResizeDepthAttachment(const cVector2& size)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    _renderTarget->SetDepthAttachment(
        gfxResourceBackend->ResizeTexture(
            _renderTarget->GetDepthAttachment(),
            size
        )
    );
}

triton::SShaderDefine triton::XRenderPass::SetInputTexture(types::usize slot, const SRenderPassTexture& texture)
{
    if (slot >= _inputTextures.size())
        return SShaderDefine("", 0);

    _inputTextures.at(slot) = texture;

    return SShaderDefine(texture._name, slot);
}

std::vector<triton::SShaderDefine> triton::XRenderPass::SetInputTextures(const std::vector<triton::SRenderPassTexture>& textures)
{
    std::vector<SShaderDefine> defines = {};
    for (usize i = 0; i < textures.size(); i++)
    {
        const usize textureAtlasTextureIndex = i;
        const std::string& textureAtlasTextureName = textures[i]._name;
        defines.push_back({ textureAtlasTextureName, textureAtlasTextureIndex });
    }

    return defines;
}