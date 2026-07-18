// TRF.hpp

#pragma once

#include <optional>
#include <filesystem>
#include "types.hpp"

namespace triton
{
	enum class EResourceFileType
	{
		Model3D
	};

	template <EResourceFileType TResourceType>
	class CResourceFile
	{
		types::usize _dataByteSize = 0;
		types::u8* _data = nullptr;
		types::usize _parsedDataByteSize = 0;
		types::u8* _parsedData = nullptr;

	public:
		CResourceFile() = delete;
		explicit CResourceFile(const std::filesystem::path& dataFilePath);
		~CResourceFile();

		void ParseData();

	private:
		void ParseModel3DData();
	};
}