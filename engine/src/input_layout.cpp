// input_layout.cpp

#include "input_layout.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XInputLayout::XInputLayout(
	cContext* context,
	const std::vector<CGPUBuffer>& buffersToBind,
	EVertexBufferFormat vertexFormat
) : iObject(context)
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_INPUT_LAYOUT
	));
	_gpuVertexArray = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUInputLayout>();

	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::BIND_INPUT_LAYOUT,
		(cpuword)&_gpuVertexArray
	));

	for (auto& buffer : buffersToBind)
		_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_BUFFER,
			(cpuword)&buffer
		));

	if (vertexFormat == EVertexBufferFormat::Static_52)
		_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_STATIC_INPUT_LAYOUT
		));
	else if (vertexFormat == EVertexBufferFormat::Skinned_84)
		_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_SKINNED_INPUT_LAYOUT
		));

	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::UNBIND_INPUT_LAYOUT
	));
}

triton::XInputLayout::~XInputLayout()
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_INPUT_LAYOUT,
		(cpuword)&_gpuVertexArray
	));
}