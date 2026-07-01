// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_buffer_formats.hpp"
#include "data_buffer.hpp"
#include "engine.hpp"
#include "render_subsystem.hpp"
#include "graphics_resource_backend.hpp"
#include "vertex.hpp"

using namespace types;

void triton::XGeometryStorage::Initialize()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::VERTEX,
        (cpuword)nullptr,
        caps->vertexBufferSize,
        0
    ));
    _vertexBuffer = renderSubsystem->FetchResult<cBuffer*>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::INDEX,
        (cpuword)nullptr,
        caps->indexBufferSize,
        0
    ));
    _indexBuffer = renderSubsystem->FetchResult<cBuffer*>();
    _vertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->vertexBufferSize);
    _indexBufferCPU = _context->Create<XDataBuffer>(_context, caps->indexBufferSize);
    
    /*_opaqueInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderOpaqueInstanceCount, 0);
    _transparentInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTransparentInstanceCount, 0);
    _textInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextInstanceCount, 0);
    _opaqueMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    _textMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextInstanceCount, 1);
    _transparentMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    _lightBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderLightCount, 2);
    _opaqueTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    _transparentTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);*/
}

void triton::XGeometryStorage::Free()
{
    _context->Destroy<XDataBuffer>(_indexBufferCPU);
    _context->Destroy<XDataBuffer>(_vertexBufferCPU);

    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_indexBuffer,
        0,
        0,
        0
    ));
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_vertexBuffer,
        0,
        0,
        0
    ));

    /*gfxResourceBackend->DestroyBuffer(_transparentTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_lightBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueInstanceBuffer);*/
}

std::optional<triton::SGeometryView> triton::XGeometryStorage::Store(EGraphicsBufferFormat format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize)
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    
    usize vertexBufferByteSize = _vertexBufferPointer;
    usize indexBufferByteSize = _indexBufferPointer;
    _vertexBufferPointer += verticesByteSize;
    _indexBufferPointer += indicesByteSize;

    usize vertexCount;
    usize indexCount = indicesByteSize / sizeof(u32);
    usize vertexElementOffset;
    usize indexElementOffset = indexBufferByteSize / sizeof(u32);
    switch (format)
    {
        case EGraphicsBufferFormat::POSITION_TEXCOORD_NORMAL_TANGENT_VEC3_VEC2_VEC3_VEC4:
        {
            const usize kVertexByteSize = sizeof(SVertex);
            vertexCount = verticesByteSize / kVertexByteSize;
            vertexElementOffset = vertexBufferByteSize / kVertexByteSize;
            break;
        }

        default:
        {
            Print("Error: unsupported vertex buffer format!");
            return std::nullopt;
        }
    }
    u8* vertexData = &_vertexBufferCPU->GetData()[vertexBufferByteSize];
    u8* indexData = &_indexBufferCPU->GetData()[indexBufferByteSize];

    // CPU write
    _vertexBufferCPU->Write(vertices, verticesByteSize, vertexBufferByteSize);
    _indexBufferCPU->Write(indices, indicesByteSize, indexBufferByteSize);

    // GPU write
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::WRITE_BUFFER,
        (cpuword)_vertexBuffer,
        vertexBufferByteSize,
        verticesByteSize,
        (cpuword)vertexData
    ));
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::WRITE_BUFFER,
        (cpuword)_indexBuffer,
        indexBufferByteSize,
        indicesByteSize,
        (cpuword)indexData
    ));

    SGeometryView geometry;
    geometry._vertexCount = vertexCount;
    geometry._indexCount = indexCount;
    geometry._vertexElementOffset = vertexElementOffset;
    geometry._indexElementOffset = indexElementOffset;
    geometry._vertexData = vertexData;
    geometry._indexData = indexData;
    geometry._format = format;

    return geometry;
}

void triton::XGeometryStorage::Bind()
{
    CThreadGuard::AssertRender();

    _vertexBuffer->Bind();
    _indexBuffer->Bind();
}