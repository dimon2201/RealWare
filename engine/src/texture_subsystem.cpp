// texture_subsystem.cpp

#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>
#include "texture_subsystem.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "application.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "log.hpp"
#include "render_subsystem.hpp"
#include "handle_allocator.hpp"
#include "filesystem_manager.hpp"

using namespace types;

triton::XTextureSubsystem::XTextureSubsystem(cContext* context, const cVector3& size) : ISubsystem(context)
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)cTexture::eDimension::TEXTURE_2D_ARRAY,
        (cpuword)cTexture::eFormat::RGBA8_SRGB_MIPS,
        (cpuword)nullptr,
        0
    ));
    _atlasRGBA8SRGB = renderSubsystem->FetchResult<cTexture*>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)cTexture::eDimension::TEXTURE_2D_ARRAY,
        (cpuword)cTexture::eFormat::RGBA8,
        (cpuword)nullptr,
        1
    ));
    _atlasRGBA8 = renderSubsystem->FetchResult<cTexture*>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::CREATE_TEXTURE,
        size.GetX(),
        size.GetY(),
        size.GetZ(),
        (cpuword)cTexture::eDimension::TEXTURE_2D_ARRAY,
        (cpuword)cTexture::eFormat::R8,
        (cpuword)nullptr,
        2
    ));
    _atlasR8 = renderSubsystem->FetchResult<cTexture*>();
}

triton::XTextureSubsystem::~XTextureSubsystem()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)_atlasR8
    ));
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)_atlasRGBA8
    ));
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)_atlasRGBA8SRGB
    ));
}

triton::HTexture triton::XTextureSubsystem::CreateTexture(const std::string& filePath, cTexture::eFormat dataFormat)
{
    auto tff = CreateTextureFromFile(dataFormat, filePath);;

    HTexture texture = Create();
    *_objects->Get(texture) = tff.has_value() ? *tff : STexture();

    return texture;
}

triton::HTexture triton::XTextureSubsystem::CreateTexture(const types::u8* byteData, types::usize byteSize, ETextureFormat fileFormat, cTexture::eFormat dataFormat)
{
    HTexture texture = Create();
    *_objects->Get(texture) = *CreateTextureFromBytes(dataFormat, byteData, byteSize, fileFormat);

    return texture;
}

std::optional<triton::STexture> triton::XTextureSubsystem::CreateTexture(cTexture::eFormat format, const cVector2& size, const types::u8* data)
{
    if (data == nullptr ||
        (format != cTexture::eFormat::RGBA8_SRGB_MIPS &&
        format != cTexture::eFormat::RGBA8 &&
        format != cTexture::eFormat::R8))
    {
        Print("Error: can't create texture with unsupported format");

        return std::nullopt;
    }

    cTexture* atlas = nullptr;
    if (format == cTexture::eFormat::RGBA8_SRGB_MIPS)
        atlas = _atlasRGBA8SRGB;
    else if (format == cTexture::eFormat::RGBA8)
        atlas = _atlasRGBA8;
    else if (format == cTexture::eFormat::R8)
        atlas = _atlasR8;

    const usize width = size.GetX();
    const usize height = size.GetY();
    const SBufferView<STexture> textureBuffer = _objects->GetData();
    const STexture* textures = textureBuffer._elements;
    const usize textureCount = textureBuffer._elementCount;
    STexture candidateTexture;
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
                    candidateTexture.layer = layer;
                    candidateTexture.normOffset = normOffset;
                    candidateTexture.normSize = normSize;
                    if (IsOverlapping(candidateTexture, textures[i]))
                    {
                        isOverlapping = K_TRUE;
                        break;
                    }
                }

                if (isOverlapping == K_FALSE)
                {
                    const cVector2 pixelOffset = cVector2(x, y);
                    const cVector2 pixelSize = size;

                    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
                    renderSubsystem->PushCommand(SRenderCommand(
                        ERenderCommand::WRITE_TEXTURE,
                        (cpuword)atlas,
                        pixelOffset.GetX(),
                        pixelOffset.GetY(),
                        layer,
                        pixelSize.GetX(),
                        pixelSize.GetY(),
                        (cpuword)data
                    ));
                    if (format == cTexture::eFormat::RGBA8_SRGB_MIPS)
                        renderSubsystem->PushCommand(SRenderCommand(
                            ERenderCommand::GENERATE_TEXTURE_MIPS,
                            (cpuword)atlas
                        ));
                    renderSubsystem->FetchResult<void*>(); // TODO: do proper synchronization here

                    STexture readyTexture;
                    readyTexture.layer = layer;
                    readyTexture.normOffset = candidateTexture.normOffset;
                    readyTexture.normSize = candidateTexture.normSize;
                    readyTexture.pixelOffset = pixelOffset;
                    readyTexture.pixelSize = pixelSize;

                    return readyTexture;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<triton::STexture> triton::XTextureSubsystem::CreateTextureFromFile(cTexture::eFormat dataFormat, const std::string& filePath)
{
    cDataFile* df = _context->GetSubsystem<cFileSystem>()->CreateDataFile(filePath, K_FALSE);
    if (!df->Exists())
        return std::nullopt;
    XDataBuffer* db = df->GetBuffer();
    if (db->GetByteSize() == 0)
        return std::nullopt;
    
    ETextureFormat tf = ETextureFormat::NONE;
    const usize kPNGMagicByteCount = 8;
    const usize kDDSMagicByteCount = 4;
    const usize kMagicByteCountMax = std::max({ kPNGMagicByteCount, kDDSMagicByteCount });
    const u8 pngMagic[kPNGMagicByteCount] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    const u8 ddsMagic[kDDSMagicByteCount] = { 0x44, 0x44, 0x53, 0x20 };
    u8 compareBuffer[kMagicByteCountMax] = {};
    memcpy(&compareBuffer[0], &db->GetData()[0], kMagicByteCountMax);
    if (memcmp(&compareBuffer[0], &pngMagic[0], kPNGMagicByteCount) == 0)
        tf = ETextureFormat::PNG;
    else if (memcmp(&compareBuffer[0], &ddsMagic[0], kDDSMagicByteCount) == 0)
        tf = ETextureFormat::DDS;

    usize width = 0;
    usize height = 0;
    usize channels = 0;
    u8* data = nullptr;

    if (tf == ETextureFormat::NONE)
    {
        Print("Error: unknown texture format, file path: '" + filePath + "'\n");

        return std::nullopt;
    }
    else if (tf == ETextureFormat::PNG)
    {
        int stbWidth = 0, stbHeight = 0, stbChannels = 0;
        
        data = stbi_load(filePath.c_str(), &stbWidth, &stbHeight, &stbChannels, 0);
        if (stbChannels == 3 && (dataFormat == cTexture::eFormat::RGBA8 ||
            dataFormat == cTexture::eFormat::RGBA8_SRGB_MIPS))
        {
            Print("Info: recreate image buffer from 3 channels to 4 channels");

            u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(stbWidth, stbHeight), data);
            stbi_image_free(data);
            auto tex = CreateTexture(dataFormat, cVector2(stbWidth, stbHeight), (const u8*)rgbaPixels);
            DestroyPixelBuffer(rgbaPixels);

            return tex;
        }
        else if (stbChannels == 3 && dataFormat == cTexture::eFormat::R8)
        {
            Print("Info: recreate image buffer from 3 channel to 1 channel");

            u8* rPixels = RecreatePixelBuffer(3, 1, cVector2(stbWidth, stbHeight), data);
            stbi_image_free(data);
            auto tex = CreateTexture(dataFormat, cVector2(stbWidth, stbHeight), (const u8*)rPixels);
            DestroyPixelBuffer(rPixels);

            return tex;
        }
        if (stbChannels == 1 && dataFormat != cTexture::eFormat::R8)
        {
            Print("Error: requested PNG texture has R8 format");
            return std::nullopt;
        }
        else if (stbChannels == 4 && dataFormat != cTexture::eFormat::RGBA8_SRGB_MIPS)
        {
            Print("Error: requested PNG texture has RGBA8 format");
            return std::nullopt;
        }

        width = stbWidth;
        height = stbHeight;
        channels = stbChannels;

        auto result = CreateTexture(dataFormat, cVector2(width, height), data);

        stbi_image_free(data);

        return result;
    }
    else if (tf == ETextureFormat::DDS)
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
            dataFormat != cTexture::eFormat::R8)
        {
            Print("Error: requested DDS texture has R8 format");
            return std::nullopt;
        }
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB &&
            dataFormat != cTexture::eFormat::RGBA8)
        {
            Print("Error: requested DDS texture has RGBA8_SRGB format");
            return std::nullopt;
        }
        if (fmt == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm &&
            dataFormat != cTexture::eFormat::RGBA8)
        {
            Print("Error: requested DDS texture has RGBA8 format");
            return std::nullopt;
        }

        width = dds.GetWidth();
        height = dds.GetHeight();
        channels = ddsChannels;
        data = (u8*)dds.GetImageData()->m_mem;

        return CreateTexture(dataFormat, cVector2(width, height), data);
    }
}

std::optional<triton::STexture> triton::XTextureSubsystem::CreateTextureFromBytes(cTexture::eFormat dataFormat, const u8* byteData, usize byteSize, ETextureFormat fileFormat)
{
    if (fileFormat != ETextureFormat::PNG)
    {
        Print("Error: can not create texture from byte data because format is not PNG format");
        return std::nullopt;
    }
    
    int width = 0, height = 0, channels = 0;
    stbi_uc* pixelsStbi = stbi_load_from_memory(
        byteData,
        byteSize,
        &width,
        &height,
        &channels,
        0
    );

    cTexture::eFormat texFmt = cTexture::eFormat::NONE;
    if (channels == 1)
    {
        texFmt = cTexture::eFormat::R8;
    }
    else if (channels == 4)
    {
        texFmt = cTexture::eFormat::RGBA8_SRGB_MIPS;
    }
    else if (channels == 3)
    {
        Print("Info: recreate image buffer from 3 channels to 4 channels");

        u8* rgbaPixels = RecreatePixelBuffer(3, 4, cVector2(width, height), pixelsStbi);
        stbi_image_free(pixelsStbi);
        auto tex = CreateTexture(cTexture::eFormat::RGBA8, cVector2(width, height), (const u8*)rgbaPixels);
        DestroyPixelBuffer(rgbaPixels);

        return tex;
    }
    else
    {
        Print("Error: unsupported PNG texture format, channel count: " + std::to_string(channels));
        return std::nullopt;
    }

    auto tex = CreateTexture(texFmt, cVector2(width, height), (const u8*)pixelsStbi);

    stbi_image_free(pixelsStbi);
    
    return tex;
}

types::boolean triton::XTextureSubsystem::IsOverlapping(const STexture& candidateTexture, const STexture& atlasTexture)
{
    if (candidateTexture.normOffset.GetX() + candidateTexture.normSize.GetX() > 1.0f ||
        candidateTexture.normOffset.GetY() + candidateTexture.normSize.GetY() > 1.0f)
        return K_TRUE;

    return
        candidateTexture.layer == atlasTexture.layer &&
        candidateTexture.normOffset.GetX() <
        atlasTexture.normOffset.GetX() + atlasTexture.normSize.GetX() &&
        candidateTexture.normOffset.GetX() + candidateTexture.normSize.GetX() >
        atlasTexture.normOffset.GetX() &&
        candidateTexture.normOffset.GetY() <
        atlasTexture.normOffset.GetY() + atlasTexture.normSize.GetY() &&
        candidateTexture.normOffset.GetY() + candidateTexture.normSize.GetY() >
        atlasTexture.normOffset.GetY();
}

types::u8* triton::XTextureSubsystem::RecreatePixelBuffer(usize srcChannelCount, usize dstChannelCount, const cVector2& size, const u8* data)
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

void triton::XTextureSubsystem::DestroyPixelBuffer(const u8* rgbaByteData)
{
    _context->GetMemoryAllocator()->Deallocate((void*)rgbaByteData);
}