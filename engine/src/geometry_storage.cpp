// geometry_storage.cpp

#include "geometry_storage.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics_buffer_formats.hpp"
#include "data_buffer.hpp"
#include "engine.hpp"
#include "vertex.hpp"
#include "synchronization.hpp"
#include "thread_guard.hpp"
#include "gpu_buffer_types.hpp"

using namespace types;

triton::XGeometryStorage::XGeometryStorage(cContext* context) : iObject(context)
{
    cEngine* engine = _context->GetSubsystem<cEngine>();
    XRenderCommandRecorder* cmdRecorder = engine->GetRenderCommandRecorder();
    XSynchronization* sync = engine->GetSynchronization();
    IApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();
    const sCapabilities* caps = app->GetCapabilities();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)EGPUBufferType::Vertex,
        (cpuword)nullptr,
        caps->staticVertexBufferSize,
        0
    ));
    _staticVertexBuffer = sync->WaitForRenderCommandResult<CGPUBuffer>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)EGPUBufferType::Vertex,
        (cpuword)nullptr,
        caps->skinnedVertexBufferSize,
        0
    ));
    _skinnedVertexBuffer = sync->WaitForRenderCommandResult<CGPUBuffer>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)EGPUBufferType::Index,
        (cpuword)nullptr,
        caps->staticIndexBufferSize,
        0
    ));
    _staticIndexBuffer = sync->WaitForRenderCommandResult<CGPUBuffer>();

    cmdRecorder->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)EGPUBufferType::Index,
        (cpuword)nullptr,
        caps->skinnedIndexBufferSize,
        0
    ));
    _skinnedIndexBuffer = sync->WaitForRenderCommandResult<CGPUBuffer>();

    _staticVertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->staticVertexBufferSize);
    _skinnedVertexBufferCPU = _context->Create<XDataBuffer>(_context, caps->skinnedVertexBufferSize);
    _staticIndexBufferCPU = _context->Create<XDataBuffer>(_context, caps->staticIndexBufferSize);
    _skinnedIndexBufferCPU = _context->Create<XDataBuffer>(_context, caps->skinnedIndexBufferSize);
}

triton::XGeometryStorage::~XGeometryStorage()
{
    _context->Destroy<XDataBuffer>(_skinnedIndexBufferCPU);
    _context->Destroy<XDataBuffer>(_staticIndexBufferCPU);
    _context->Destroy<XDataBuffer>(_skinnedVertexBufferCPU);
    _context->Destroy<XDataBuffer>(_staticVertexBufferCPU);

    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&_skinnedIndexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&_staticIndexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&_skinnedVertexBuffer,
        0,
        0,
        0
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&_staticVertexBuffer,
        0,
        0,
        0
    ));
}

std::optional<triton::SGeometryView> triton::XGeometryStorage::Create(
    EVertexBufferFormat format,
    const u8* vertices,
    usize inputVertexCount,
    const u8* indices,
    usize inputIndexCount
)
{
    if (format == EVertexBufferFormat::Static_52)
    {
        const usize verticesByteSize = sizeof(SStaticVertexGPULayout) * inputVertexCount;
        const usize indicesByteSize = sizeof(u32) * inputIndexCount;

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
            (cpuword)&_staticVertexBuffer,
            vertexBufferByteSize,
            verticesByteSize,
            (cpuword)vertexData
        ));
        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::WRITE_BUFFER,
            (cpuword)&_staticIndexBuffer,
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
    else if (format == EVertexBufferFormat::Skinned_84)
    {
        const usize verticesByteSize = sizeof(SSkinnedVertexGPULayout) * inputVertexCount;
        const usize indicesByteSize = sizeof(u32) * inputIndexCount;

        usize vertexBufferByteSize = _skinnedVertexBufferPointer;
        usize indexBufferByteSize = _skinnedIndexBufferPointer;
        _skinnedVertexBufferPointer += verticesByteSize;
        _skinnedIndexBufferPointer += indicesByteSize;

        const usize cVertexByteSize = sizeof(SSkinnedVertexGPULayout);
        const usize cIndexByteSize = sizeof(u32);
        usize vertexCount = verticesByteSize / cVertexByteSize;
        usize indexCount = indicesByteSize / cIndexByteSize;
        usize vertexElementOffset = vertexBufferByteSize / cVertexByteSize;
        usize indexElementOffset = indexBufferByteSize / cIndexByteSize;

        u8* vertexData = &_skinnedVertexBufferCPU->GetData()[vertexBufferByteSize];
        u8* indexData = &_skinnedIndexBufferCPU->GetData()[indexBufferByteSize];
        _skinnedVertexBufferCPU->Write(vertices, verticesByteSize, vertexBufferByteSize);
        _skinnedIndexBufferCPU->Write(indices, indicesByteSize, indexBufferByteSize);

        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::WRITE_BUFFER,
            (cpuword)&_skinnedVertexBuffer,
            vertexBufferByteSize,
            verticesByteSize,
            (cpuword)vertexData
        ));
        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
            ERenderCommand::WRITE_BUFFER,
            (cpuword)&_skinnedIndexBuffer,
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
    
    return std::nullopt;
}