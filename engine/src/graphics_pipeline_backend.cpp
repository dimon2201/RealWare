// graphics_pipeline_backend.cpp

#include "graphics_pipeline_backend.hpp"
#include "context.hpp"
#include "render_subsystem.hpp"
#include "graphics.hpp"

using namespace types;

triton::XShader::XShader(cContext* context, const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFuncStr, const std::string& fragmentCustomFuncStr, std::vector<SShaderDefine>&& defines) : iObject(context)
{
	XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
	/*renderSubsystem->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_SHADER,
		(cpuword)vertexStr.c_str(),
		(cpuword)fragmentStr.c_str(),
		(cpuword)vertexCustomFuncStr.c_str(),
		(cpuword)fragmentCustomFuncStr.c_str(),
		defines.size(),
		(cpuword)defines.data()
	));
	_gpuShader = renderSubsystem->FetchResult<CGPUShader>();*/
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
	iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
	iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
	_gpuVertexArray = gfxPipelineBackend->CreateVertexArray();
	gfxPipelineBackend->BindVertexArray(_gpuVertexArray);
	for (auto buffer : buffersToBind)
		gfxResourceBackend->BindBuffer(buffer);
	gfxPipelineBackend->BindDefaultInputLayout();
	gfxPipelineBackend->UnbindVertexArray();
}

triton::XVertexArray::~XVertexArray()
{
	iGraphicsPipelineBackend* gfxPipelineBackend = _context->GetBackend<iGraphicsPipelineBackend>();
	gfxPipelineBackend->DestroyVertexArray(_gpuVertexArray);
}

triton::XRenderTarget::XRenderTarget(
	cContext* context,
	qword instance,
	const std::vector<cTexture*>& colorAttachments,
	cTexture* depthAttachment
) : cGPUResource(context, instance, 0), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment) {}

triton::iGraphicsPipelineBackend::iGraphicsPipelineBackend(cContext* context) : iBackend(context) {}