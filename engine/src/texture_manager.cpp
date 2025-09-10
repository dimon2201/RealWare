#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "application.hpp"
#include "texture_manager.hpp"
#include "render_context.hpp"

namespace realware
{
    using namespace core;

    namespace render
    {
        mTexture::mTexture(const cApplication* const app, const cRenderContext* const context)
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

        sTextureAtlasTexture* mTexture::AddTexture(const std::string& filename, const std::string& tag)
        {
            int width = 0, height = 0, channels = 0;
            unsigned char* data = nullptr;
            data = stbi_load(filename.data(), &width, &height, &channels, 4);

            if (data == nullptr || channels < 3)
                return nullptr;

            if (channels == 3)
            {
                unsigned char* dataTemp = (unsigned char*)malloc(width * height * 4);

                for (s32 i = 0; i < width * height; i++)
                {
                    dataTemp[(i * 4)] = data[(i * 3)];
                    dataTemp[(i * 4) + 1] = data[(i * 3) + 1];
                    dataTemp[(i * 4) + 2] = data[(i * 3) + 2];
                    dataTemp[(i * 4) + 3] = 255;
                }

                free(data);

                data = dataTemp;
            }

            for (int layer = 0; layer < _atlas->Depth; layer++)
            {
                for (int y = 0; y < _atlas->Height; y++)
                {
                    for (int x = 0; x < _atlas->Width; x++)
                    {
                        bool isIntersecting = false;

                        for (auto& area : _textures)
                        {
                            if (area->IsNormalized == core::K_FALSE)
                            {
                                const glm::vec4 textureRect = glm::vec4(
                                    x, y, x + width, y + height
                                );
                                if ((area->Offset.z == layer &&
                                    area->Offset.x <= textureRect.z && area->Offset.x + area->Size.x >= textureRect.x &&
                                    area->Offset.y <= textureRect.w && area->Offset.y + area->Size.y >= textureRect.y) ||
                                    (x + width > _atlas->Width || y + height > _atlas->Height))
                                {
                                    isIntersecting = true;
                                    break;
                                }
                            }
                            else if (area->IsNormalized == core::K_TRUE)
                            {
                                const glm::vec4 textureRectNorm = glm::vec4(
                                    (float)x / (float)_atlas->Width, (float)y / (float)_atlas->Height,
                                    ((float)x + (float)width) / (float)_atlas->Width, ((float)y + (float)height) / (float)_atlas->Height
                                );
                                if ((area->Offset.z == layer &&
                                    area->Offset.x <= textureRectNorm.z && area->Offset.x + area->Size.x >= textureRectNorm.x &&
                                    area->Offset.y <= textureRectNorm.w && area->Offset.y + area->Size.y >= textureRectNorm.y) ||
                                    (textureRectNorm.z > 1.0f || textureRectNorm.w > 1.0f))
                                {
                                    isIntersecting = true;
                                    break;
                                }
                            }
                        }

                        if (!isIntersecting)
                        {
                            sTextureAtlasTexture* area = new sTextureAtlasTexture();
                            area->Tag = tag;
                            area->Offset = glm::vec3(x, y, layer);
                            area->Size = glm::vec2(width, height);

                            _textures.push_back(area);

                            _context->WriteTexture(_atlas, area->Offset, area->Size, data);
                            if (_atlas->Format == render::sTexture::eFormat::RGBA8_MIPS)
                                _context->GenerateTextureMips(_atlas);

                            free(data);

                            return area;
                        }
                    }
                }
            }

            free(data);

            return nullptr;
        }

        void mTexture::DeleteTexture(const std::string& tag)
        {
            for (auto it = _textures.begin(); it != _textures.end(); it++)
            {
                if ((*it)->Tag == tag)
                {
                    _textures.erase(it);
                    return;
                }
            }
        }

        /*void mTexture::LoadAnimation(const std::vector<const std::string&>& filenames, const std::string& tag, std::vector<sArea*>& frames)
        {
            usize i = 0;
            for (auto filename : filenames)
            {
                sTextureAtlasTexture* area = AddTexture(filename, tag);
                if (area != nullptr)
                {
                    *area = CalculateNormalizedArea(*area);
                    i += 1;

                    frames.emplace_back(area);
                }
            }

            _animations.emplace_back(frames);
        }

        void mTexture::PlayAnimation(entity object, cScene* scene, float speed)
        {
            sCAnimation* animation = scene->Get<sCAnimation>(object);

            if (animation->Tick >= animation->MaxTick) {
                animation->Tick = 0.0f;
            }

            animation->Tick += speed;
        }

        void mTexture::IncrementAnimationFrame(entity object, cScene* scene)
        {
            sCAnimation* animation = scene->Get<sCAnimation>(object);

            animation->CurrentFrameIndex[animation->CurrentAnimationIndex] += 1;
            if (animation->CurrentFrameIndex[animation->CurrentAnimationIndex] >= animation->Frames[animation->CurrentAnimationIndex]->size()) {
                animation->CurrentFrameIndex[animation->CurrentAnimationIndex] = 0;
            }
        }

        void mTexture::RemoveAnimation(const std::string& tag)
        {
            for (auto it = _textures.begin(); it != _textures.end();)
            {
                if ((*it)->Tag == tag)
                {
                    _textures.erase(it);
                }
                else
                {
                    it++;
                }
            }

            for (auto it = _animations.begin(); it != _animations.end(); it++)
            {
                if ((*it)[0]->Tag == tag)
                {
                    _animations.erase(it);
                    return;
                }
            }
        }*/

        sTextureAtlasTexture* mTexture::GetTexture(const std::string& tag)
        {
            for (auto& area : _textures) {
                if (area->Tag == tag)
                    return area;
            }

            return nullptr;
        }

        std::vector<sTextureAtlasTexture*>* mTexture::GetAnimation(const std::string& tag)
        {
            for (auto& animation : _animations) {
                for (auto& area : animation) {
                    if (area->Tag == tag)
                        return &animation;
                }
            }

            return nullptr;
        }

        sTextureAtlasTexture mTexture::CalculateNormalizedArea(const sTextureAtlasTexture& area)
        {
            sTextureAtlasTexture a;
            a.Tag = area.Tag;
            a.IsNormalized = K_TRUE;
            a.Offset.x = area.Offset.x / _atlas->Width;
            a.Offset.y = area.Offset.y / _atlas->Height;
            a.Offset.z = area.Offset.z;
            a.Size.x = area.Size.x / _atlas->Width;
            a.Size.y = area.Size.y / _atlas->Height;

            return a;
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