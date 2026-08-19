// shader.cpp

#include "shader.hpp"
#include "context.hpp"
#include "engine.hpp"
#include "synchronization.hpp"
#include "graphics_backend.hpp"
#include "thread_guard.hpp"

using namespace types;

triton::XShader::XShader(
	cContext* context,
	const std::string& vertexStr,
	const std::string& fragmentStr,
	const std::string& vertexCustomFuncStr,
	const std::string& fragmentCustomFuncStr,
	const std::vector<SShaderDefine>& defines,
	const std::vector<const char*>& vertexIncludePaths,
	const std::vector<const char*>& fragmentIncludePaths
) : iObject(context)
{
	CThreadGuard::AssertMain();

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
	_gpuShader = _context->GetSubsystem<cEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUShaderResource>();
}

triton::XShader::~XShader()
{
	CThreadGuard::AssertMain();

	_context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_SHADER,
		(cpuword)&_gpuShader
	));
	_context->GetSubsystem<cEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<void*>();
}