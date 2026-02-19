// texture_manager.cpp

#include "texture_manager.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "application.hpp"
#include "graphics_backend_facade.hpp"
#include "memory_pool.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "log.hpp"

using namespace types;

triton::cTextureAtlasTexture::cTextureAtlasTexture(cContext* context, types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size, cTexture* atlas)
    : iObject(context), _isNormalized(isNormalized)
{
    if (isNormalized == K_TRUE)
    {
        _offset = glm::vec3(offset.x / atlas->GetWidth(), offset.y / atlas->GetHeight(), offset.z);
        _size = glm::vec2(size.x / atlas->GetWidth(), size.y / atlas->GetHeight());
    }
    else
    {
        _offset = offset;
        _size = size;
    }
}

triton::cTextureAtlas::cTextureAtlas(cContext* context) : iObject(context) {}

void triton::cTextureAtlas::Initialize(const cVector3& size)
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    _atlas = gfxResourceBackend->CreateTexture(
        cVector3(size.GetX(), size.GetY(), size.GetZ()),
        cTexture::eDimension::TEXTURE_2D_ARRAY,
        cTexture::eFormat::RGBA8_MIPS,
        nullptr,
        0
    );
    _atlas->SetSlot(0);
}

void triton::cTextureAtlas::Shutdown()
{
    iGraphicsResourceBackend* gfxResourceBackend = _context->GetBackend<iGraphicsResourceBackend>();
    if (_atlas)
        gfxResourceBackend->DestroyTexture(_atlas);
}

// TODO: New implementation of texture creation
/*cTextureAtlasTexture* cTextureAtlas::CreateTexture(const std::string& id, const glm::vec2& size, usize channels, const u8* data)
{
    const usize width = size.x;
    const usize height = size.y;

    if (data == nullptr || channels != 4)
    {
        Print("Error: you can only create texture with 4 channels in RGBA format!");

        return nullptr;
    }

    const auto textures = _textures.GetElements();
    const usize texturesCount = _textures.GetElementCount();
    for (usize layer = 0; layer < _atlas->GetDepth(); layer++)
    {
        for (usize y = 0; y < _atlas->GetHeight(); y++)
        {
            for (usize x = 0; x < _atlas->GetWidth(); x++)
            {
                types::boolean isIntersecting = K_FALSE;

                for (usize i = 0; i < texturesCount; i++)
                {
                    const auto& area = textures[i];

                    if (area.IsNormalized() == K_FALSE)
                    {
                        const glm::vec4 textureRect = glm::vec4(
                            x, y, x + width, y + height
                        );
                        if ((area.GetOffset().z == layer &&
                            area.GetOffset().x <= textureRect.z && area.GetOffset().x + area.GetSize().x >= textureRect.x &&
                            area.GetOffset().y <= textureRect.w && area.GetOffset().y + area.GetSize().y >= textureRect.y) ||
                            (x + width > _atlas->GetWidth() || y + height > _atlas->GetHeight()))
                        {
                            isIntersecting = K_FALSE;
                            break;
                        }
                    }
                    else if (area.IsNormalized() == K_TRUE)
                    {
                        const glm::vec4 textureRectNorm = glm::vec4(
                            (f32)x / (f32)_atlas->GetWidth(), (f32)y / (f32)_atlas->GetHeight(),
                            ((f32)x + (f32)width) / (f32)_atlas->GetWidth(), ((f32)y + (f32)height) / (f32)_atlas->GetHeight()
                        );
                        if ((area.GetOffset().z == layer &&
                            area.GetOffset().x <= textureRectNorm.z && area.GetOffset().x + area.GetSize().x >= textureRectNorm.x &&
                            area.GetOffset().y <= textureRectNorm.w && area.GetOffset().y + area.GetSize().y >= textureRectNorm.y) ||
                            (textureRectNorm.z > 1.0f || textureRectNorm.w > 1.0f))
                        {
                            isIntersecting = true;
                            break;
                        }
                    }
                }

                if (!isIntersecting)
                {
                    const glm::vec3 offset = glm::vec3(x, y, layer);
                    const glm::vec2 size = glm::vec2(width, height);

                    _gfx->WriteTexture(_atlas, offset, size, data);
                    if (_atlas->GetFormat() == cTexture::eFormat::RGBA8_MIPS)
                        _gfx->GenerateTextureMips(_atlas);

                    cTextureAtlasTexture* newTex = _textures.Add(_context, K_TRUE, offset, size);

                    return newTex;
                }
            }
        }
    }

    return nullptr;
}

cTextureAtlasTexture* cTextureAtlas::CreateTexture(const std::string& id, const std::string& filename)
{
    const usize channelsRequired = 4;

    s32 width = 0;
    s32 height = 0;
    s32 channels = 0;
    u8* data = nullptr;
    data = stbi_load(filename.c_str(), &width, &height, &channels, channelsRequired);

    return CreateTexture(id, glm::vec2(width, height), channelsRequired, data);
}

cTextureAtlasTexture* cTextureAtlas::FindTexture(const std::string& id)
{
    return _textures.Find(id);
}

void cTextureAtlas::DestroyTexture(const std::string& id)
{
    _textures.Delete(id);
}*/

triton::cTexture* triton::cTextureAtlas::GetAtlas() const
{
    return _atlas;
}

usize triton::cTextureAtlas::GetWidth() const
{
    return _atlas->GetWidth();
}

usize triton::cTextureAtlas::GetHeight() const
{
    return _atlas->GetHeight();
}

usize triton::cTextureAtlas::GetDepth() const
{
    return _atlas->GetDepth();
}