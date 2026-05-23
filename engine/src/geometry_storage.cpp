// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_resource_backend.hpp"
#include "graphics_buffer_formats.hpp"
#include "buffer.hpp"

using namespace types;

void triton::XGeometryStorage::Initialize()
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();

    _vertexBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::VERTEX, nullptr, caps->vertexBufferSize, 0);
    _indexBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::INDEX, nullptr, caps->indexBufferSize, 0);
    _opaqueInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderOpaqueInstanceCount, 0);
    _transparentInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTransparentInstanceCount, 0);
    _textInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextInstanceCount, 0);
    _opaqueMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    _textMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextInstanceCount, 1);
    _transparentMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderMaterialCount, 1);
    _lightBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderLightCount, 2);
    _opaqueTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    _transparentTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::STORAGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    _vertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->vertexBufferSize);
    _indexBufferCPU = _context->Create<XDataBuffer>(_context, caps->indexBufferSize);
}

void triton::XGeometryStorage::Free()
{
    _context->Destroy<XDataBuffer>(_indexBufferCPU);
    _context->Destroy<XDataBuffer>(_vertexBufferCPU);

    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    gfxResourceBackend->DestroyBuffer(_transparentTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueTextureAtlasTexturesBuffer);
    gfxResourceBackend->DestroyBuffer(_lightBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueMaterialBuffer);
    gfxResourceBackend->DestroyBuffer(_textInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_transparentInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_opaqueInstanceBuffer);
    gfxResourceBackend->DestroyBuffer(_indexBuffer);
    gfxResourceBackend->DestroyBuffer(_vertexBuffer);
}

std::optional<triton::SGeometryView> triton::XGeometryStorage::CreateGeometry(EGraphicsBufferFormat format, const types::u8* vertices, types::usize verticesByteSize, const types::u8* indices, types::usize indicesByteSize)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    
    usize vertexBufferByteSize = _vertexBufferPointer;
    usize indicesBufferByteSize = _indexBufferPointer;

    // CPU write
    _vertexBufferCPU->Write(vertices, verticesByteSize, _vertexBuffer->GetByteSize());
    _indexBufferCPU->Write(indices, indicesByteSize, _indexBuffer->GetByteSize());

    // GPU write
    gfxResourceBackend->WriteBuffer(_vertexBuffer, vertexBufferByteSize, verticesByteSize, vertices);
    gfxResourceBackend->WriteBuffer(_indexBuffer, indicesByteSize, indicesByteSize, indices);

    usize vertexCount = verticesByteSize;
    usize vertexOffset = vertexBufferByteSize;
    usize indexOffset = indicesBufferByteSize;
    
    _vertexBufferPointer += verticesByteSize;
    _indexBufferPointer += indicesByteSize;

    usize indexCount = indicesByteSize / sizeof(u32);
    u8* vertexAddress = &_vertexBufferCPU->GetData()[vertexOffset];
    u8* indexAddress = &_indexBufferCPU->GetData()[indexOffset];

    switch (format)
    {
    case EGraphicsBufferFormat::VERTEX_POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3:
        vertexCount /= 32;
        vertexOffset /= 32;
        break;

    default:
        Print("Error: unsupported vertex buffer format!");
        return std::nullopt;
    }

    // Return geometry view
    SGeometryView geometry;
    geometry._vertexCount = vertexCount;
    geometry._indexCount = indexCount;
    geometry._vertexAddress = vertexAddress;
    geometry._indexAddress = indexAddress;
    geometry._format = format;

    return geometry;
}