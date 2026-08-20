// gpu_buffer.cpp

#include "gpu_buffer.hpp"
#include "context.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XGPUBuffer::XGPUBuffer(
    cContext* context,
    s32 poolIndex,
    EGPUBufferType type,
    const u8* data,
    usize byteSize,
    s32 slot
) : iObject(context, poolIndex)
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_BUFFER,
        (cpuword)type,
        (cpuword)data,
        byteSize,
        slot
    ));
    _gpuBuffer =
        _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<CGPUBufferResource>();
}

triton::XGPUBuffer::~XGPUBuffer()
{
    _context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_BUFFER,
        (cpuword)&_gpuBuffer
    ));
    _context->GetSubsystem<CEngine>()->
        GetSynchronization()->
        WaitForRenderCommandResult<void*>();
}