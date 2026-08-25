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
	s32 poolIndex,
	types::dword stageMask,
	const SShaderBytecodeFiles& bytecodeFiles
) : iObject(context, poolIndex)
{
	CThreadGuard::AssertMain();

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::CREATE_SHADER,
		(cpuword)stageMask,
		(cpuword)&bytecodeFiles
	));

	_gpuShader = _context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<CGPUShaderResource>();
}

triton::XShader::~XShader()
{
	CThreadGuard::AssertMain();

	_context->GetSubsystem<CEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
		ERenderCommand::DESTROY_SHADER,
		(cpuword)&_gpuShader
	));

	_context->GetSubsystem<CEngine>()->
		GetSynchronization()->
		WaitForRenderCommandResult<void*>();
}