// texture.hpp

#pragma once

#include <filesystem>
#include "object.hpp"
#include "math.hpp"
#include "gpu_texture_resource.hpp"
#include "texture_formats.hpp"
#include "texture_dimensions.hpp"
#include "image_format_enum.hpp"
#include "image_file_format_enum.hpp"
#include "handle.hpp"

namespace triton
{
	class cContext;

	class XTexture : public iObject
	{
		TRITON_CLASS_NAME(XTexture)

		CGPUTextureResource _gpuTexture = CGPUTextureResource::Invalid();

	public:
		explicit XTexture(
			cContext* context,
			types::s32 poolIndex,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			types::s32 textureSlot,
			EImageFormat expectedDataFormat,
			const std::filesystem::path& filePath
		);

		explicit XTexture(
			cContext* context,
			types::s32 poolIndex,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			types::s32 textureSlot,
			EImageFileFormat containerFormat,
			EImageFormat expectedDataFormat,
			const types::u8* data,
			types::usize dataByteSize
		);

		explicit XTexture(
			cContext* context,
			types::s32 poolIndex,
			types::boolean bCreateSampler,
			ETextureFormat textureFormat,
			types::dword usageMask,
			ETextureDimension textureDimension,
			const cVector3& size,
			const types::u8* data,
			types::s32 textureSlot
		);

		~XTexture() override;

		inline const CGPUTextureResource& GetGPUResource() const { return _gpuTexture; }

		inline const cVector3& GetSize() const { return cVector3(_gpuTexture.GetWidth(), _gpuTexture.GetHeight(), _gpuTexture.GetDepth()); }

		inline ETextureFormat GetFormat() const { return _gpuTexture.GetFormat(); }

		inline types::s32 GetSlot() const { return _gpuTexture.GetSlot(); }

		struct THandle : public SHandle {};

		struct TGPULayout {};
		
	private:
		void CreateOnGpu(
			types::boolean bCreateSampler,
			ETextureFormat format,
			types::dword usageMask,
			ETextureDimension dimension,
			const cVector3& size,
			const types::u8* data,
			types::s32 slot
		);

		void GenerateMips();
	};
}