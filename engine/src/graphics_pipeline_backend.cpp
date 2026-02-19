// graphics_pipeline_backend.cpp

#include "graphics_pipeline_backend.hpp"
#include "graphics_texture_backend.hpp"
#include "context.hpp"

using namespace types;

triton::cShader::sDefinePair::sDefinePair(const std::string& name_, types::usize index_) : name(name_), index(index_) {}

triton::cShader::cShader(
	cContext* context,
	qword instance,
	const std::string& vertexShaderStr,
	const std::string& fragmentShaderStr
)
: cGPUResource(context, instance, 0), _vertex(vertexShaderStr), _fragment(fragmentShaderStr) {}

triton::cVertexArray::cVertexArray(cContext* context, qword instance) : cGPUResource(context, instance, 0) {}

triton::cRenderPassGPU::cRenderPassGPU(cContext* context, cVertexArray* vertexArray, cShader* shader)
	: iObject(context), _vertexArray(vertexArray), _shader(shader) {}

triton::cRenderTarget::cRenderTarget(
	cContext* context,
	qword instance,
	const std::vector<cTexture*>& colorAttachments,
	cTexture* depthAttachment
)
: cGPUResource(context, instance, 0), _colorAttachments(colorAttachments), _depthAttachment(depthAttachment) {}

triton::iGraphicsPipelineBackend::iGraphicsPipelineBackend(cContext* context) : iBackend(context) {}