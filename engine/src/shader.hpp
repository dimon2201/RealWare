// shader.hpp

#pragma once

#include <string>
#include <vector>
#include "object.hpp"
#include "handle.hpp"
#include "gpu_shader_resource.hpp"
#include "shader_define.hpp"
#include "shader_bytecode_files_struct.hpp"
#include "types.hpp"

namespace triton
{
	class XShader : public iObject
	{
		TRITON_OBJECT(XShader)

		CGPUShaderResource _gpuShader = CGPUShaderResource::Invalid();

	public:
		explicit XShader(
			cContext* context,
			types::s32 poolIndex,
			types::dword stageMask,
			const SShaderBytecodeFiles& bytecodeFiles
		);

		~XShader() override;

		inline const CGPUShaderResource& GetGPUResource() const { return _gpuShader; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}