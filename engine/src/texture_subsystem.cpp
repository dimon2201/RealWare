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
        (cpuword)cTexture::eFormat::RGBA8_MIPS,
        (cpuword)nullptr,
        0
    ));
    _atlas = renderSubsystem->FetchResult<cTexture*>();
}

triton::XTextureSubsystem::~XTextureSubsystem()
{
    XRenderSubsystem* renderSubsystem = _context->GetSubsystem<XRenderSubsystem>();
    renderSubsystem->PushCommand(SRenderCommand(
        ERenderCommand::DESTROY_TEXTURE,
        (cpuword)_atlas
    ));
}

triton::HTexture triton::XTextureSubsystem::CreateTexture(const std::string& filePath)
{
    HTexture texture = Create();
    *_objects->Get(texture) = *CreateTextureFromFile(filePath);

    return texture;
}

std::optional<triton::STexture> triton::XTextureSubsystem::CreateTexture(cTexture::eFormat format, const cVector2& size, const types::u8* data)
{
    if (data == nullptr || (format != cTexture::eFormat::RGBA8 && format != cTexture::eFormat::RGBA8_MIPS))
    {
        Print("Error: you can only create texture atlas with 4 channels in RGBA format!");

        return std::nullopt;
    }

    const usize width = size.GetX();
    const usize height = size.GetY();
    const SBufferView<STexture> textureBuffer = _objects->GetData();
    const STexture* textures = textureBuffer._elements;
    const usize textureCount = textureBuffer._elementCount;
    STexture candidateTexture;
    for (usize layer = 0; layer < _atlas->GetDepth(); layer++)
    {
        for (usize y = 0; y < _atlas->GetHeight(); y++)
        {
            for (usize x = 0; x < _atlas->GetWidth(); x++)
            {
                const cVector2 normOffset = cVector2((f32)x / _atlas->GetWidth(), (f32)y / _atlas->GetHeight());
                const cVector2 normSize = cVector2(size.GetX() / _atlas->GetWidth(), size.GetY() / _atlas->GetHeight());

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
                        (cpuword)_atlas,
                        pixelOffset.GetX(),
                        pixelOffset.GetY(),
                        layer,
                        pixelSize.GetX(),
                        pixelSize.GetY(),
                        (cpuword)data
                    ));
                    if (format == cTexture::eFormat::RGBA8_MIPS)
                        renderSubsystem->PushCommand(SRenderCommand(
                            ERenderCommand::GENERATE_TEXTURE_MIPS,
                            (cpuword)_atlas
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

std::optional<triton::STexture> triton::XTextureSubsystem::CreateTextureFromFile(const std::string& filePath)
{
    cDataFile* df = _context->GetSubsystem<cFileSystem>()->CreateDataFile(filePath, K_FALSE);
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
        int stbWidth = width;
        int stbHeight = height;
        int stbChannels = channels;
        const int channelsRequired = 4;

        data = stbi_load(filePath.c_str(), &stbWidth, &stbHeight, &stbChannels, channelsRequired);

        width = stbWidth;
        height = stbHeight;
        channels = stbChannels;
    }
    else if (tf == ETextureFormat::DDS)
    {
        tinyddsloader::DDSFile dds;

        auto ret = dds.Load(filePath.c_str());
        auto fmt = dds.GetFormat();
        if (fmt != tinyddsloader::DDSFile::DXGIFormat::R8G8B8A8_UNorm)
        {
            Print("Error: unsupported DDS texture format, file path: '" + filePath + "'\n");
            return std::nullopt;
        }

        width = dds.GetWidth();
        height = dds.GetHeight();
        channels = 4;
        data = (u8*)dds.GetImageData()->m_mem;

        return CreateTexture(cTexture::eFormat::RGBA8_MIPS, cVector2(width, height), data);
    }

    return CreateTexture(cTexture::eFormat::RGBA8_MIPS, cVector2(width, height), data);
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