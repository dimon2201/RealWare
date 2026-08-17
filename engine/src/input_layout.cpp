// input_layout.cpp

#include "input_layout.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XInputLayout::XInputLayout(
	cContext* context,
	const std::vector<cBuffer*>& buffersToBind,
	EVertexBufferFormat vertexFormat
) : iObject(context)
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_VERTEX_ARRAY
	));
	_gpuVertexArray = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUVertexArray>();

	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::BIND_VERTEX_ARRAY,
		(cpuword)&_gpuVertexArray
	));

	for (auto buffer : buffersToBind)
		_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
			ERenderCommand::BIND_BUFFER,
			(cpuword)buffer
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
		ERenderCommand::UNBIND_VERTEX_ARRAY
	));
}

triton::XInputLayout::~XInputLayout()
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_VERTEX_ARRAY,
		(cpuword)&_gpuVertexArray
	));
}