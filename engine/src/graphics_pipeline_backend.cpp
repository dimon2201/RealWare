// graphics_pipeline_backend.cpp

#include "graphics_pipeline_backend.hpp"
#include "context.hpp"
#include "render_subsystem.hpp"
#include "graphics.hpp"

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
	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	renderSubsystem->PushCommand(SRenderCommand(
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
	_gpuShader = renderSubsystem->FetchResult<CGPUShader>();
}

triton::XShader::~XShader()
{
	_context->GetSubsystem<XRenderSubsystem>()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_SHADER,
		(cpuword)&_gpuShader
	));
}

triton::XVertexArray::XVertexArray(cContext* context, const std::vector<cBuffer*>& buffersToBind) : iObject(context)
{
	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	cGraphics* gfx = _context->GetSubsystem<cGraphics>();

	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_VERTEX_ARRAY
	));
	_gpuVertexArray = renderSubsystem->FetchResult<CGPUVertexArray>();

	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::BIND_VERTEX_ARRAY,
		(cpuword)&_gpuVertexArray
	));

	for (auto buffer : buffersToBind)
		renderSubsystem->PushCommand(SRenderCommand(
			ERenderCommand::BIND_BUFFER,
			(cpuword)buffer
		));

	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::BIND_DEFAULT_INPUT_LAYOUT
	));

	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::UNBIND_VERTEX_ARRAY
	));
}

triton::XVertexArray::~XVertexArray()
{
	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_VERTEX_ARRAY,
		(cpuword)&_gpuVertexArray
	));
}

triton::XRenderTarget::XRenderTarget(
	cContext* context,
	qword instance,
	const std::vector<cTexture*>& colorAttachments,
	cTexture* depthAttachment
) : cGPUResource(context, instance, 0), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment) {}

triton::iGraphicsPipelineBackend::iGraphicsPipelineBackend(cContext* context) : iBackend(context) {}