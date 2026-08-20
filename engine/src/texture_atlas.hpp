// texture_atlas.hpp

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include "object.hpp"
#include "math.hpp"
#include "atlas_texture.hpp"
#include "graphics_backend.hpp"
#include "texture_file_formats.hpp"
#include "subsystem.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class iGraphicsBackend;
    class cApplication;

    class CTextureAtlas final : public CSubsystem
    {
        TRITON_CLASS_NAME(CTextureAtlas)

        CGPUTextureResource _atlasRGBA8SRGB;
        CGPUTextureResource _atlasRGBA8;
        CGPUTextureResource _atlasR8;
        std::vector<STextureAtlasRegion> _texturesRGBA8SRGB;
        std::vector<STextureAtlasRegion> _texturesRGBA8;
        std::vector<STextureAtlasRegion> _texturesR8;

    public:
        explicit CTextureAtlas(cContext* context, const cVector3& size);
        ~CTextureAtlas();

        std::optional<STextureAtlasRegion> Create(
            const std::filesystem::path& filePath,
            ETextureFormat dataFormat
        );
        
        std::optional<STextureAtlasRegion> Create(
            const types::u8* byteData,
            types::usize byteDataByteSize,
            types::usize width,
            types::usize height,
            types::usize channelCount,
            ETextureFileFormat byteDataFormat,
            ETextureFormat pixelDataFormat
        );

        inline CGPUTextureResource GetAtlasRGBA8SRGB() const
        {
            return _atlasRGBA8SRGB;
        }

        inline CGPUTextureResource GetAtlasRGBA8() const
        {
            return _atlasRGBA8;
        }

        inline CGPUTextureResource GetAtlasR8() const
        {
            return _atlasR8;
        }

    private:
        std::optional<STextureAtlasRegion> CreateTextureOnAtlas(
            ETextureFormat format,
            const cVector2& size,
            const types::u8* data
        );
        
        std::optional<STextureAtlasRegion> CreateTextureOnAtlasFromFile(
            ETextureFormat dataFormat,
            const std::string& filePath
        );
        
        std::optional<STextureAtlasRegion> CreateTextureOnAtlasFromBytes(
            ETextureFormat expectedDataFormat,
            const types::u8* byteData,
            types::usize byteDataByteSize,
            types::usize width,
            types::usize height,
            types::usize channelCount,
            ETextureFileFormat fileFormat
        );
        
        types::boolean IsOverlapping(
            const STextureAtlasRegion& candidateTexture,
            const STextureAtlasRegion& atlasTexture
        );
        
        types::u8* RecreatePixelBuffer(
            types::usize srcChannelCount,
            types::usize dstChannelCount,
            const cVector2& size,
            const types::u8* data
        );
        
        void DestroyPixelBuffer(const types::u8* rgbaByteData);
    };
}