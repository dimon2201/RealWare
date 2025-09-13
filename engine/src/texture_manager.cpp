#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "application.hpp"
#include "texture_manager.hpp"
#include "render_context.hpp"

using namespace types;

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace utils;

    namespace render
    {
        cTextureAtlasTexture::cTextureAtlasTexture(const types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size) : _isNormalized(isNormalized), _offset(offset), _size(size)
        {
        }

        mTexture::mTexture(const cApplication* const app, const cRenderContext* const context) : _textures((cApplication*)app, ((cApplication*)app)->GetDesc()->MaxTextureCount)
        {
            _app = (cApplication*)app;

            sApplicationDescriptor* desc = _app->GetDesc();

            _context = (cRenderContext*)context;
            _atlas = _context->CreateTexture(
                desc->TextureAtlasWidth,
                desc->TextureAtlasHeight,
                desc->TextureAtlasDepth,
                sTexture::eType::TEXTURE_2D_ARRAY,
                sTexture::eFormat::RGBA8_MIPS,
                nullptr
            );
            _atlas->Slot = 0;
        }

        mTexture::~mTexture()
        {
            _context->DestroyTexture(_atlas);
        }

        cTextureAtlasTexture* mTexture::AddTexture(const std::string& id, const std::string& filename)
        {
            s32 width = 0;
            s32 height = 0;
            s32 channels = 0;
            u8* data = nullptr;
            data = stbi_load(filename.data(), &width, &height, &channels, 4);

            if (data == nullptr || channels < 3)
                return nullptr;

            if (channels == 3)
            {
                u8* const dataTemp = (u8* const)malloc(width * height * 4);

                for (usize i = 0; i < width * height; i++)
                {
                    dataTemp[(i * 4)] = data[(i * 3)];
                    dataTemp[(i * 4) + 1] = data[(i * 3) + 1];
                    dataTemp[(i * 4) + 2] = data[(i * 3) + 2];
                    dataTemp[(i * 4) + 3] = 255;
                }

                free(data);

                data = dataTemp;
            }

            const auto& textures = _textures.GetObjects();
            for (usize layer = 0; layer < _atlas->Depth; layer++)
            {
                for (usize y = 0; y < _atlas->Height; y++)
                {
                    for (usize x = 0; x < _atlas->Width; x++)
                    {
                        types::boolean isIntersecting = K_FALSE;

                        for (auto& area : textures)
                        {
                            if (area.IsNormalized() == K_FALSE)
                            {
                                const glm::vec4 textureRect = glm::vec4(
                                    x, y, x + width, y + height
                                );
                                if ((area.GetOffset().z == layer &&
                                    area.GetOffset().x <= textureRect.z && area.GetOffset().x + area.GetSize().x >= textureRect.x &&
                                    area.GetOffset().y <= textureRect.w && area.GetOffset().y + area.GetSize().y >= textureRect.y) ||
                                    (x + width > _atlas->Width || y + height > _atlas->Height))
                                {
                                    isIntersecting = K_FALSE;
                                    break;
                                }
                            }
                            else if (area.IsNormalized() == K_TRUE)
                            {
                                const glm::vec4 textureRectNorm = glm::vec4(
                                    (f32)x / (f32)_atlas->Width, (f32)y / (f32)_atlas->Height,
                                    ((f32)x + (f32)width) / (f32)_atlas->Width, ((f32)y + (f32)height) / (f32)_atlas->Height
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

                            _context->WriteTexture(_atlas, offset, size, data);
                            if (_atlas->Format == render::sTexture::eFormat::RGBA8_MIPS)
                                _context->GenerateTextureMips(_atlas);

                            free(data);

                            return _textures.Add(id, K_FALSE, offset, size);
                        }
                    }
                }
            }

            free(data);

            return nullptr;
        }

        cTextureAtlasTexture* mTexture::FindTexture(const std::string& id)
        {
            return _textures.Find(id);
        }

        void mTexture::DeleteTexture(const std::string& id)
        {
            _textures.Delete(id);
        }

        cTextureAtlasTexture mTexture::CalculateNormalizedArea(const cTextureAtlasTexture& area)
        {
            cTextureAtlasTexture norm = cTextureAtlasTexture(
                types::K_TRUE,
                glm::vec3(area.GetOffset().x / _atlas->Width, area.GetOffset().y / _atlas->Height, area.GetOffset().z),
                glm::vec2(area.GetSize().x / _atlas->Width, area.GetSize().y / _atlas->Height)
            );

            return norm;
        }

        sTexture* mTexture::GetAtlas()
        {
            return _atlas;
        }

        usize mTexture::GetWidth() const
        {
            return _atlas->Width;
        }

        usize mTexture::GetHeight() const
        {
            return _atlas->Height;
        }

        usize mTexture::GetDepth() const
        {
            return _atlas->Depth;
        }
    }
}