// graphics_pipeline_backend.cpp

#include "graphics_pipeline_backend.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "synchronization.hpp"

using namespace types;

triton::XShader::XShader(
	cContext* context,
	const std::string& vertexStr,
	const std::string& fragmentStr,
	const std::string& vertexCustomFuncStr,
	const std::string& fragmentCustomFuncStr,
	std::vector<SShaderDefine>&& defines,
	const std::vector<const char*>& vertexIncludePaths,
	const std::vector<const char*>& fragmentIncludePaths
) : iObject(context)
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_SHADER,
		(cpuword)vertexStr.c_str(),
		(cpuword)fragmentStr.c_str(),
		(cpuword)vertexCustomFuncStr.c_str(),
		(cpuword)fragmentCustomFuncStr.c_str(),
		defines.size(),
		(cpuword)defines.data(),
		vertexIncludePaths.size(),
		(cpuword)vertexIncludePaths.data(),
		fragmentIncludePaths.size(),
		(cpuword)fragmentIncludePaths.data()
	));
	_gpuShader = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUShader>();
}

triton::XShader::~XShader()
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_SHADER,
		(cpuword)&_gpuShader
	));
}

triton::XVertexArray::XVertexArray(
	cContext* context,
	const std::vector<cBuffer*>& buffersToBind,
	EVertexBufferFormat vertexFormat
): iObject(context)
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

triton::XVertexArray::~XVertexArray()
{
	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_VERTEX_ARRAY,
		(cpuword)&_gpuVertexArray
	));
}

triton::XRenderTargetBackend::XRenderTargetBackend(
	cContext* context,
	qword instance,
	const std::vector<cTexture*>& colorAttachments,
	cTexture* depthAttachment
) : cGPUResource(context, instance, 0), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment) {}

triton::iGraphicsPipelineBackend::iGraphicsPipelineBackend(cContext* context) : iBackend(context) {}