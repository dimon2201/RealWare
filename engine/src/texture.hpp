// texture.hpp

#pragma once

#include <filesystem>
#include "object.hpp"
#include "math.hpp"
#include "gpu_texture_resource.hpp"
#include "image_file_format_enum.hpp"
#include "handle.hpp"

namespace triton
{
	class cContext;

	class XTexture : public iObject
	{
		TRITON_CLASS_NAME(XTexture)

		CGPUTextureResource _gpuTexture;

	public:
		explicit XTexture(
			cContext* context,
			types::s32 poolIndex,
			ETextureFormat expectedDataFormat,
			const std::filesystem::path& filePath
		);

		explicit XTexture(
			cContext* context,
			types::s32 poolIndex,
			EImageFileFormat containerFormat,
			ETextureFormat expectedDataFormat,
			const types::u8* byteData,
			types::usize byteDataByteSize
		);

		~XTexture() override;

		inline const CGPUTextureResource& GetGPUResource() const { return _gpuTexture; }

		inline const cVector3& GetSize() const { return cVector3(_gpuTexture.GetWidth(), _gpuTexture.GetHeight(), _gpuTexture.GetDepth()); }

		inline ETextureFormat GetFormat() const { return _gpuTexture.GetFormat(); }

		inline types::s32 GetSlot() const { return _gpuTexture.GetSlot(); }

		struct THandle : public SHandle {};

		struct TGPULayout {};
	};
}