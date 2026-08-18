// texture_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include <optional>
#include "object.hpp"
#include "math.hpp"
#include "linear_array.hpp"
#include "texture_data.hpp"
#include "graphics_backend.hpp"
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class iGraphicsBackend;
    class cApplication;
    class XTexturePool;

    struct sTextureAtlasTextureGPU
    {
        glm::vec4 _textureInfo = glm::vec4(0.0f);
        types::f32 _textureLayerInfo = 0.0f;
    };

    enum class ETextureFileFormat
    {
        NONE,
        Raw,
        PNG,
        DDS
    };

    class XTextureSubsystem : public ISubsys
    {
        TRITON_OBJECT(XTextureSubsystem)

        XTexturePool* _pool = nullptr;
        CGPUTexture _atlasRGBA8SRGB;
        CGPUTexture _atlasRGBA8;
        CGPUTexture _atlasR8;

    public:
        explicit XTextureSubsystem(cContext* context, const cVector3& size);
        ~XTextureSubsystem() override;

        std::optional<STextureData::THandle> Create(const std::string& filePath, ETextureFormat dataFormat);
        std::optional<STextureData::THandle> Create(
            const types::u8* byteData,
            types::usize byteDataByteSize,
            types::usize width,
            types::usize height,
            types::usize channelCount,
            ETextureFileFormat byteDataFormat,
            ETextureFormat pixelDataFormat
        );
        void Init() override {}
        void Free() override {}
        void Update() override {}

        inline CGPUTexture GetAtlasRGBA8SRGB() const
        {
            return _atlasRGBA8SRGB;
        }

        inline CGPUTexture GetAtlasRGBA8() const
        {
            return _atlasRGBA8;
        }

        inline CGPUTexture GetAtlasR8() const
        {
            return _atlasR8;
        }

        inline XTexturePool* GetPool() const
        {
            return _pool;
        }

    private:
        std::optional<STextureData> CreateTexture(ETextureFormat format, const cVector2& size, const types::u8* data);
        std::optional<STextureData> CreateTextureFromFile(ETextureFormat dataFormat, const std::string& filePath);
        std::optional<STextureData> CreateTextureFromBytes(
            ETextureFormat expectedDataFormat,
            const types::u8* byteData,
            types::usize byteDataByteSize,
            types::usize width,
            types::usize height,
            types::usize channelCount,
            ETextureFileFormat fileFormat
        );
        types::boolean IsOverlapping(const STextureData& candidateTexture, const STextureData& atlasTexture);
        types::u8* RecreatePixelBuffer(types::usize srcChannelCount, types::usize dstChannelCount, const cVector2& size, const types::u8* data);
        void DestroyPixelBuffer(const types::u8* rgbaByteData);
    };
}