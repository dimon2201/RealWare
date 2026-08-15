// asset_loader.hpp

#pragma once

#include <optional>
#include <filesystem>
#include "object.hpp"
#include "model3d_data.hpp"

namespace triton
{
	class cContext;
	class XModel3DPool;

	class XAssetLoader : public iObject
	{
		TRITON_OBJECT(XAssetLoader)

		XModel3DPool* _model3dPool = nullptr;

	public:
		explicit XAssetLoader(cContext* context);
		~XAssetLoader() override;

		std::optional<SModel3DData::THandle> LoadModel(const std::filesystem::path& path);
	};
}