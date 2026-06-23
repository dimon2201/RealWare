// graphics_pipeline_backend.cpp

#include "graphics_pipeline_backend.hpp"
#include "context.hpp"
#include "render_subsystem.hpp"

using namespace types;

triton::XShader::XShader(cContext* context, const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFuncStr, const std::string& fragmentCustomFuncStr, std::vector<SShaderDefine>&& defines) : iObject(context)
{
	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_SHADER,
		(cpuword)vertexStr.c_str(),
		(cpuword)fragmentStr.c_str(),
		(cpuword)vertexCustomFuncStr.c_str(),
		(cpuword)fragmentCustomFuncStr.c_str(),
		defines.size(),
		(cpuword)defines.data()
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

triton::CVertexArray::CVertexArray(cContext* context, qword instance) : cGPUResource(context, instance, 0) {}

triton::XRenderTarget::XRenderTarget(
	cContext* context,
	qword instance,
	const std::vector<cTexture*>& colorAttachments,
	cTexture* depthAttachment
)
: cGPUResource(context, instance, 0), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment) {}

triton::iGraphicsPipelineBackend::iGraphicsPipelineBackend(cContext* context) : iBackend(context) {}