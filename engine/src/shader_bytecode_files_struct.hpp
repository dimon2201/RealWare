// shader_bytecode_files_struct.hpp

#pragma once

#include <optional>
#include <filesystem>

namespace triton
{
	struct SShaderBytecodeFiles final
	{
		std::optional<std::filesystem::path> vertexFilePath = std::nullopt;
		std::optional<std::filesystem::path> pixelFilePath = std::nullopt;
		std::optional<std::filesystem::path> tessellationControlFilePath = std::nullopt;
		std::optional<std::filesystem::path> tessellationEvaluationFilePath = std::nullopt;
		std::optional<std::filesystem::path> computeFilePath = std::nullopt;
	};
}