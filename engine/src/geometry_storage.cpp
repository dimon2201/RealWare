// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_resource_backend.hpp"
#include "graphics_buffer_formats.hpp"
#include "buffer.hpp"

void triton::XGeometryStorage::Initialize()
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();

    _vertexBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::VERTEX, nullptr, caps->vertexBufferSize, 0);
    _indexBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::INDEX, nullptr, caps->indexBufferSize, 0);
    _opaqueInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderOpaqueInstanceCount, 0);
    _transparentInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderTransparentInstanceCount, 0);
    _textInstanceBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderTextInstanceCount, 0);
    _opaqueMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderMaterialCount, 1);
    _textMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderTextInstanceCount, 1);
    _transparentMaterialBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderMaterialCount, 1);
    _lightBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderLightCount, 2);
    _opaqueTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    _transparentTextureAtlasTexturesBuffer = gfxResourceBackend->CreateBuffer(cBuffer::eType::LARGE, nullptr, caps->maxRenderTextureAtlasTextureCount, 3);
    _vertexBufferCPU = _context->Create<cDataBuffer>(_context, caps->vertexBufferSize);
    _indexBufferCPU = _context->Create<cDataBuffer>(_context, caps->indexBufferSize);
}

void triton::XGeometryStorage::Free()
{
    _context->Destroy<cDataBuffer>(_indexBufferCPU);
    _context->Destroy<cDataBuffer>(_vertexBufferCPU);

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

triton:::SGeometryView triton::XGeometryStorage::CreateGeometry(EGraphicsBufferFormat format, const u8* vertices, usize verticesByteSize, const u8* indices, usize indicesByteSize)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    
    // CPU write
    _vertexBuffer->Write(vertices, verticesByteSize, _verticesByteSize);
    _indexBuffer->Write(indices, indicesByteSize, _indicesByteSize);

    // GPU write
    gfxResourceBackend->WriteBuffer(_vertexBuffer, _verticesByteSize, verticesByteSize, vertices);
    gfxResourceBackend->WriteBuffer(_indexBuffer, _indicesByteSize, indicesByteSize, indices);

    usize vertexCount = verticesByteSize;
    usize vertexOffset = _verticesByteSize;
    usize indexOffset = _indicesByteSize;
    
    _verticesByteSize += verticesByteSize;
    _indicesByteSize += indicesByteSize;

    usize indexCount = indicesByteSize / sizeof(u32);
    u8* vertexAddress = &_vertices->GetData()[vertexOffset];
    u8* indexAddress = &_index->GetData()[indexOffset];

    switch (format)
    {
    case eCategory::VERTEX_BUFFER_FORMAT_POS_TEX_NRM_VEC3_VEC2_VEC3:
        vertexCount /= 32;
        vertexOffset /= 32;
        break;

    default:
        Print("Error: unsupported vertex buffer format!");
        return nullptr;
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