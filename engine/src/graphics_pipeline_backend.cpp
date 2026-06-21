// graphics_pipeline_backend.cpp

#include "graphics_resource_backend.hpp"
#include "graphics_pipeline_backend.hpp"
#include "context.hpp"

using namespace types;

triton::XShader::XShader(cContext* context, const std::string& vertexStr, const std::string& fragmentStr, const std::string& vertexCustomFuncStr, const std::string& fragmentCustomFuncStr, const std::vector<SShaderDefine>&& defines) : iObject(context)
{
	_gpuShader = _context->GetBackend<iGraphicsPipelineBackend>()->CreateShader(
		vertexStr,
		fragmentStr,
		vertexCustomFuncStr,
		fragmentCustomFuncStr,
		defines
	);
}

triton::XShader::~XShader()
{
	_context->GetBackend<iGraphicsPipelineBackend>()->DestroyShader(_gpuShader);
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