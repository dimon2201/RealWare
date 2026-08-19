// texture_atlas.cpp

#include "texture_atlas.hpp"
#include <stb_image.h>
#include <tinyddsloader.h>
#include "application.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "log.hpp"
#include "handle_allocator.hpp"
#include "filesystem_manager.hpp"

using namespace types;

triton::XTextureAtlas::XTextureAtlas(cContext* context, const cVector3& size) : ISubsys(context)
{
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)ETextureDimension::Texture2DArray,
        (cpuword)ETextureFormat::RGBA8_SRGB_Mips,
        (cpuword)nullptr,
        0
    ));
    _atlasRGBA8SRGB = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTexture>();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)ETextureDimension::Texture2DArray,
        (cpuword)ETextureFormat::RGBA8,
        (cpuword)nullptr,
        1
    ));
    _atlasRGBA8 = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTexture>();
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)ETextureDimension::Texture2DArray,
        (cpuword)ETextureFormat::R8,
        (cpuword)nullptr,
        2
    ));
    _atlasR8 = _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<CGPUTexture>();
}

triton::XTextureAtlas::~XTextureAtlas()
{
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&_atlasR8
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&_atlasRGBA8
    ));
    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)&_atlasRGBA8SRGB
    ));
}

std::optional<triton::STextureAtlasRegion> triton::XTextureAtlas::Create(
    const std::filesystem::path& filePath,
    ETextureFormat dataFormat
)
{
    return CreateTextureOnAtlasFromFile(dataFormat, filePath.generic_string());
}

std::optional<triton::STextureAtlasRegion> triton::XTextureAtlas::Create(
    const u8* byteData,
    usize byteDataByteSize,
    usize width,
    usize height,
    usize channelCount,
    ETextureFileFormat byteDataFormat,
    ETextureFormat pixelDataFormat
)
{
    return CreateTextureOnAtlasFromBytes(
        pixelDataFormat,
        byteData,
        byteDataByteSize,
        width,
        height,
        channelCount,
        byteDataFormat
    );
}

std::optional<triton::STextureAtlasRegion> triton::XTextureAtlas::CreateTextureOnAtlas(
    ETextureFormat format,
    const cVector2& size,
    const types::u8* data
)
{
    if (data == nullptr ||
        (format != ETextureFormat::RGBA8_SRGB_Mips &&
            format != ETextureFormat::RGBA8 &&
            format != ETextureFormat::R8))
    {
        Print("Error: can't create texture with unsupported format");

        return std::nullopt;
    }

    CGPUTexture* atlas = nullptr;
    std::vector<STextureAtlasRegion>* textures = nullptr;
    if (format == ETextureFormat::RGBA8_SRGB_Mips)
    {
        atlas = &_atlasRGBA8SRGB;
        textures = &_texturesRGBA8SRGB;
    }
    else if (format == ETextureFormat::RGBA8)
    {
        atlas = &_atlasRGBA8;
        textures = &_texturesRGBA8;
    }
    else if (format == ETextureFormat::R8)
    {
        atlas = &_atlasR8;
        textures = &_texturesR8;
    }

    STextureAtlasRegion candidateTexture;
    const usize width = size.GetX();
    const usize height = size.GetY();
    const usize textureCount = textures->size();
    for (usize layer = 0; layer < atlas->GetDepth(); layer++)
    {
        for (usize y = 0; y < atlas->GetHeight(); y++)
        {
            for (usize x = 0; x < atlas->GetWidth(); x++)
            {
                const cVector2 normOffset = cVector2((f32)x / atlas->GetWidth(), (f32)y / atlas->GetHeight());
                const cVector2 normSize = cVector2(size.GetX() / atlas->GetWidth(), size.GetY() / atlas->GetHeight());

                types::boolean isOverlapping = K_FALSE;
                for (usize i = 0; i < textureCount; i++)
                {
                    candidateTexture.zAtlasLayer = layer;
                    candidateTexture.offsetNorm = normOffset;
                    candidateTexture.sizeNorm = normSize;
                    if (IsOverlapping(candidateTexture, textures->at(i)))
                    {
                        isOverlapping = K_TRUE;
                        break;
                    }
                }

                if (isOverlapping == K_FALSE)
                {
                    const cVector2 pixelOffset = cVector2(x, y);
                    const cVector2 pixelSize = size;

                    _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                        ERenderCommand::WRITE_TEXTURE,
                        (cpuword)atlas,
                        pixelOffset.GetX(),
                        pixelOffset.GetY(),
                        layer,
                        pixelSize.GetX(),
                        pixelSize.GetY(),
                        (cpuword)data
                    ));
                    if (format == ETextureFormat::RGBA8_SRGB_Mips)
                        _context->GetSubsystem<cEngine>()->GetRenderCommandRecorder()->PushCommand(SRenderCommand(
                            ERenderCommand::GENERATE_TEXTURE_MIPS,
                            (cpuword)atlas
                        ));
                    _context->GetSubsystem<cEngine>()->GetSynchronization()->WaitForRenderCommandResult<void*>(); // TODO: do proper synchronization here

                    STextureAtlasRegion readyTexture;
                    readyTexture.zAtlasLayer = layer;
                    readyTexture.offsetNorm = normOffset;
                    readyTexture.sizeNorm = normSize;
                    readyTexture.offsetPixel = pixelOffset;
                    readyTexture.sizePixel = pixelSize;

                    textures->push_back(readyTexture);

                    return readyTexture;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<triton::STextureAtlasRegion> triton::XTextureAtlas::CreateTextureOnAtlasFromFile(
    ETextureFormat dataFormat,
    const std::string& filePath
)
{
    cDataFile* df = _context->GetSubsystem<cFileSystem>()->CreateDataFile(filePath, K_FALSE);
    if (!df->Exists())
        return std::nullopt;
    XDataBuffer* db = df->GetBuffer();
    if (db->GetByteSize() == 0)
        return std::nullopt;
    
    ETextureFileFormat tf = ETextureFileFormat::Unknown;
    const usize kPNGMagicByteCount = 8;
    const usize kDDSMagicByteCount = 4;
    const usize kMagicByteCountMax = std::max({ kPNGMagicByteCount, kDDSMagicByteCount });
    const u8 pngMagic[kPNGMagicByteCount] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    const u8 ddsMagic[kDDSMagicByteCount] = { 0x44, 0x44, 0x53, 0x20 };
    u8 compareBuffer[kMagicByteCountMax] = {};
    memcpy(&compareBuffer[0], &db->GetData()[0], kMagicByteCountMax);
    if (memcmp(&compareBuffer[0], &pngMagic[0], kPNGMagicByteCount) == 0)
        tf = ETextureFileFormat::PNG;
    else if (memcmp(&compareBuffer[0], &ddsMagic[0], kDDSMagicByteCount) == 0)
        tf = ETextureFileFormat::DDS;

    usize width = 0;
    usize height = 0;
    usize channels = 0;
    u8* data = nullptr;

    if (tf == ETextureFileFormat::Unknown)
    {
        Print("Error: unknown texture format, file path: '" + filePath + "'\n");

        return std::nullopt;
    }
    else if (tf == ETextureFileFormat::PNG)
    {
        int stbWidth = 0, stbHeight = 0, stbChannels = 0;
        
        data = stbi_load(filePath.c_str(), &stbWidth, &stbHeight, &stbChannels, 0);
        if (stbChannels == 3 &&
            (dataFormat == ETextureFormat::RGBA8 ||
            dataFormat == ETextureFormat::RGBA8_SRGB_Mips))
        {
            Print("Info: recreate image buffer from 3 channels to 4 channels");

            u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(stbWidth, stbHeight), data);
            stbi_image_free(data);
            auto tex = CreateTextureOnAtlas(dataFormat, cVector2(stbWidth, stbHeight), (const u8*)rgbaPixels);
            DestroyPixelBuffer(rgbaPixels);

            return tex;
        }
        else if (stbChannels == 3 && dataFormat == ETextureFormat::R8)
        {
            Print("Info: recreate image buffer from 3 channel to 1 channel");

            u8* rPixels = RecreatePixelBuffer(3, 1, cVector2(stbWidth, stbHeight), data);
            stbi_image_free(data);
            auto tex = CreateTextureOnAtlas(dataFormat, cVector2(stbWidth, stbHeight), (const u8*)rPixels);
            DestroyPixelBuffer(rPixels);

            return tex;
        }
        if (stbChannels == 1 && dataFormat != ETextureFormat::R8)
        {
            Print("Error: requested PNG texture has R8 format");
            return std::nullopt;
        }
        else if (stbChannels == 4 && dataFormat != ETextureFormat::RGBA8_SRGB_Mips)
        {
            Print("Error: requested PNG texture has RGBA8 format");
            return std::nullopt;
        }

        width = stbWidth;
        height = stbHeight;
        channels = stbChannels;

        auto result = CreateTextureOnAtlas(dataFormat, cVector2(width, height), data);

        stbi_image_free(data);

        return result;
    }
    else if (tf == ETextureFileFormat::DDS)
    {
        tinyddsloader::DDSFile dds;

        auto ret = dds.Load(filePath.c_str());
        auto fmt = dds.GetFormat();
        usize ddsChannels = 0;
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::R8_UNorm)
        {
            ddsChannels = 1;
        }
        else if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB)
        {
            ddsChannels = 4;
        }
        else if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm)
        {
            ddsChannels = 4;
        }
        else
        {
            Print("Error: unsupported DDS texture format, file path: '" + filePath + "'\n");
            return std::nullopt;
        }
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::R8_UNorm &&
            dataFormat != ETextureFormat::R8)
        {
            Print("Error: requested DDS texture has R8 format");
            return std::nullopt;
        }
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB &&
            dataFormat != ETextureFormat::RGBA8)
        {
            Print("Error: requested DDS texture has RGBA8_SRGB format");
            return std::nullopt;
        }
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm &&
            dataFormat != ETextureFormat::RGBA8)
        {
            Print("Error: requested DDS texture has RGBA8 format");
            return std::nullopt;
        }

        width = dds.GetWidth();
        height = dds.GetHeight();
        channels = ddsChannels;
        data = (u8*)dds.GetImageData()->m_mem;

        return CreateTextureOnAtlas(dataFormat, cVector2(width, height), data);
    }
}

std::optional<triton::STextureAtlasRegion> triton::XTextureAtlas::CreateTextureOnAtlasFromBytes(
    ETextureFormat expectedPixelDataFormat,
    const u8* byteData,
    usize byteDataByteSize,
    usize width,
    usize height,
    usize channelCount,
    ETextureFileFormat byteDataFormat
)
{
    u8* rawPixels;
    usize rawChannelCount = channelCount;
    ETextureFormat rawDataFormat = ETextureFormat::Unknown;
    boolean bIsPixelBufferRecreated = K_FALSE;

    if (byteDataFormat == ETextureFileFormat::Raw)
    {
        rawPixels = (u8*)byteData;

        if (expectedPixelDataFormat == ETextureFormat::RGBA8_SRGB_Mips &&
            channelCount != 4)
        {
            Print("Info: recreate image buffer from 3 channels to 4 channels");

            u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(width, height), byteData);
            free((void*)byteData);
            rawPixels = rgbaPixels;
            rawChannelCount = 4;
            rawDataFormat = ETextureFormat::RGBA8_SRGB_Mips;
        }
        else if (expectedPixelDataFormat == ETextureFormat::RGBA8 &&
            channelCount != 4)
        {
            Print("Info: recreate image buffer from 3 channels to 4 channels");

            u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(width, height), byteData);
            free((void*)byteData);
            rawPixels = rgbaPixels;
            rawChannelCount = 4;
            rawDataFormat = ETextureFormat::RGBA8;
        }
        else if (expectedPixelDataFormat == ETextureFormat::R8 &&
            channelCount == 1)
        {
            rawPixels = (u8*)byteData;
            rawChannelCount = 1;
            rawDataFormat = ETextureFormat::R8;
        }
    }
    else if (byteDataFormat == ETextureFileFormat::PNG)
    {
        int stbWidth = 0, stbHeight = 0, stbChannels = 0;
        rawPixels = stbi_load_from_memory(
            byteData,
            byteDataByteSize,
            &stbWidth,
            &stbHeight,
            &stbChannels,
            0
        );
        if (stbChannels == 1)
        {
            rawChannelCount = stbChannels;
            rawDataFormat = ETextureFormat::R8;
        }
        else if (stbChannels == 4)
        {
            rawChannelCount = stbChannels;
            rawDataFormat = ETextureFormat::RGBA8_SRGB_Mips;
        }
        else if (stbChannels == 3)
        {
            Print("Info: recreate image buffer from 3 channels to 4 channels");

            u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(stbWidth, stbHeight), rawPixels);
            stbi_image_free((void*)rawPixels);
            rawPixels = rgbaPixels;
            rawChannelCount = 4;
            rawDataFormat = ETextureFormat::RGBA8_SRGB_Mips;
            bIsPixelBufferRecreated = K_TRUE;
        }
    }
    else if (byteDataFormat == ETextureFileFormat::DDS)
    {
        tinyddsloader::DDSFile dds;
        dds.Load(byteData, width * height * channelCount);
        auto ddsFormat = dds.GetFormat();
        if (ddsFormat == tinyddsloader::DDSFile::DXGIFormat::R8_UNorm)
        {
            rawChannelCount = 1;
            rawDataFormat = ETextureFormat::R8;
        }
        else if (ddsFormat == tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB)
        {
            rawChannelCount = 4;
            rawDataFormat = ETextureFormat::RGBA8_SRGB_Mips;
        }
    }

    if (rawDataFormat != expectedPixelDataFormat)
    {
        Print("Error: current texture data format != expected texture data format, channel count: " + std::to_string(rawChannelCount));
        return std::nullopt;
    }

    auto tex = CreateTextureOnAtlas(expectedPixelDataFormat, cVector2(width, height), (const u8*)rawPixels);

    if (byteDataFormat == ETextureFileFormat::PNG && bIsPixelBufferRecreated == K_FALSE)
        stbi_image_free((void*)rawPixels);

    return tex;
}

types::boolean triton::XTextureAtlas::IsOverlapping(
    const STextureAtlasRegion& candidateTexture,
    const STextureAtlasRegion& atlasTexture
)
{
    if (candidateTexture.offsetNorm.GetX() + candidateTexture.sizeNorm.GetX() > 1.0f ||
        candidateTexture.offsetNorm.GetY() + candidateTexture.sizeNorm.GetY() > 1.0f)
        return K_TRUE;

    return
        candidateTexture.zAtlasLayer == atlasTexture.zAtlasLayer &&
        candidateTexture.offsetNorm.GetX() <
        atlasTexture.offsetNorm.GetX() + atlasTexture.sizeNorm.GetX() &&
        candidateTexture.offsetNorm.GetX() + candidateTexture.sizeNorm.GetX() >
        atlasTexture.offsetNorm.GetX() &&
        candidateTexture.offsetNorm.GetY() <
        atlasTexture.offsetNorm.GetY() + atlasTexture.sizeNorm.GetY() &&
        candidateTexture.offsetNorm.GetY() + candidateTexture.sizeNorm.GetY() >
        atlasTexture.offsetNorm.GetY();
}

types::u8* triton::XTextureAtlas::RecreatePixelBuffer(usize srcChannelCount, usize dstChannelCount, const cVector2& size, const u8* data)
{
    const usize copyChannels = std::min(srcChannelCount, dstChannelCount);
    const usize dstPixelCount = size.GetX() * size.GetY();
    u8* buffer = (u8*)_context->GetMemoryAllocator()->Allocate(dstPixelCount * dstChannelCount, 64);
    for (usize i = 0; i < dstPixelCount; i++)
    {
        for (usize j = 0; j < copyChannels; ++j)
            buffer[i * dstChannelCount + j] = data[i * srcChannelCount + j];
        for (usize j = copyChannels; j < dstChannelCount; ++j)
            buffer[i * dstChannelCount + j] = (j == 3) ? 255 : 0;
    }

    return buffer;
}

void triton::XTextureAtlas::DestroyPixelBuffer(const u8* rgbaByteData)
{
    _context->GetMemoryAllocator()->Deallocate((void*)rgbaByteData);
}