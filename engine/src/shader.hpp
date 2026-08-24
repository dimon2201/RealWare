// shader.hpp

#pragma once

#include <string>
#include <vector>
#include "object.hpp"
#include "handle.hpp"
#include "gpu_shader_resource.hpp"
#include "shader_define.hpp"
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
			const std::string& vertexStr,
			const std::string& fragmentStr,
			const std::string& vertexCustomFuncStr,
			const std::string& fragmentCustomFuncStr,
			const std::vector<SShaderDefine>& defines,
			const std::vector<const char*>& vertexIncludePaths,
			const std::vector<const char*>& fragmentIncludePaths
		);

		~XShader() override;

		inline const CGPUShaderResource& GetGPUResource() const { return _gpuShader; }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}