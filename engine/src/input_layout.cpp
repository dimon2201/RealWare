// input_layout.cpp

#include "input_layout.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "gpu_buffer_pool.hpp"

using namespace types;

triton::XInputLayout::XInputLayout(
	cContext* context,
	s32 poolIndex,
	const std::vector<XGPUBuffer::THandle>& buffersToBind,
	EVertexBufferFormat vertexFormat
) : iObject(context, poolIndex)
{
	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_INPUT_LAYOUT
	));
	_gpuInputLayout = _context->GetSubsystem<CEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUInputLayoutResource>();

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::BIND_INPUT_LAYOUT,
		(cpuword)&_gpuInputLayout
	));

	for (auto& bufferHandle : buffersToBind)
	{
		XGPUBuffer& buffer = *_context->GetPool<CGPUBufferPool>()->Get(bufferHandle);
		_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_BUFFER,
			(cpuword)&buffer.GetGPUResource()
		));
	}

	if (vertexFormat == EVertexBufferFormat::Rigid_48)
		_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_RIGID_INPUT_LAYOUT
		));
	else if (vertexFormat == EVertexBufferFormat::Skinned_80)
		_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_SKINNED_INPUT_LAYOUT
		));

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::UNBIND_INPUT_LAYOUT
	));
}

triton::XInputLayout::~XInputLayout()
{
	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_INPUT_LAYOUT,
		(cpuword)&_gpuInputLayout
	));
}