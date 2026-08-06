// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_buffer_formats.hpp"
#include "data_buffer.hpp"
#include "engine.hpp"
#include "graphics_resource_backend.hpp"
#include "vertex.hpp"
#include "synchronization.hpp"
#include "thread_guard.hpp"

using namespace types;

void triton::XGeometryStorage::Initialize()
{
    cEngine* engine = _context->GetSubsystem<cEngine>();
    XRenderCommandRecorder* cmdRecorder = engine->GetRenderCommandRecorder();
    XSynchronization* sync = engine->GetSynchronization();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::VERTEX,
        (cpuword)nullptr,
        caps->staticVertexBufferSize,
        0
    ));
    _staticVertexBuffer = sync->WaitForRenderCommandResult<cBuffer*>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::VERTEX,
        (cpuword)nullptr,
        caps->skinnedVertexBufferSize,
        0
    ));
    _skinnedVertexBuffer = sync->WaitForRenderCommandResult<cBuffer*>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::INDEX,
        (cpuword)nullptr,
        caps->staticIndexBufferSize,
        0
    ));
    _staticIndexBuffer = sync->WaitForRenderCommandResult<cBuffer*>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)cBuffer::eType::INDEX,
        (cpuword)nullptr,
        caps->skinnedIndexBufferSize,
        0
    ));
    _skinnedIndexBuffer = sync->WaitForRenderCommandResult<cBuffer*>();

    _staticVertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->staticVertexBufferSize);
    _skinnedVertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->skinnedVertexBufferSize);
    _staticIndexBufferCPU = _context->Create<XDataBuffer>(_context, caps->staticIndexBufferSize);
    _skinnedIndexBufferCPU = _context->Create<XDataBuffer>(_context, caps->skinnedIndexBufferSize);
}

void triton::XGeometryStorage::Free()
{
    _context->Destroy<XDataBuffer>(_skinnedIndexBufferCPU);
    _context->Destroy<XDataBuffer>(_staticIndexBufferCPU);
    _context->Destroy<XDataBuffer>(_skinnedVertexBufferCPU);
    _context->Destroy<XDataBuffer>(_staticVertexBufferCPU);

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinnedIndexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_staticIndexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_skinnedVertexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)_staticVertexBuffer,
        0,
        0,
        0
    ));
}

std::optional<triton::SGeometryView> triton::XGeometryStorage::Create(
    EVertexBufferFormat format,
    const u8* vertices,
    usize verticesByteSize,
    const u8* indices,
    usize indicesByteSize
)
{
    if (format == EVertexBufferFormat::Static_36)
    {
        usize vertexBufferByteSize = _staticVertexBufferPointer;
        usize indexBufferByteSize = _staticIndexBufferPointer;
        _staticVertexBufferPointer += verticesByteSize;
        _staticIndexBufferPointer += indicesByteSize;

        const usize cVertexByteSize = sizeof(SStaticVertexGPULayout);
        const usize cIndexByteSize = sizeof(u32);
        usize vertexCount = verticesByteSize / cVertexByteSize;
        usize indexCount = indicesByteSize / cIndexByteSize;
        usize vertexElementOffset = vertexBufferByteSize / cVertexByteSize;
        usize indexElementOffset = indexBufferByteSize / cIndexByteSize;
        
        u8* vertexData = &_staticVertexBufferCPU->GetData()[vertexBufferByteSize];
        u8* indexData = &_staticIndexBufferCPU->GetData()[indexBufferByteSize];
        _staticVertexBufferCPU->Write(vertices, verticesByteSize, vertexBufferByteSize);
        _staticIndexBufferCPU->Write(indices, indicesByteSize, indexBufferByteSize);

        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::WRITE_BUFFER,
            (cpuword)_staticVertexBuffer,
            vertexBufferByteSize,
            verticesByteSize,
            (cpuword)vertexData
        ));
        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::WRITE_BUFFER,
            (cpuword)_staticIndexBuffer,
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
}