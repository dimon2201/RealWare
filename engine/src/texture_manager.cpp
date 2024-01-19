#include <lodepng.h>
#define STB_IMAGE_IMPLEMENTATION
#include "../../thirdparty/stb-master/stb_image.h"
#include "texture_manager.hpp"
#include "ecs.hpp"
#include "render_context.hpp"

namespace realware
{
    namespace core
    {
        void mTexture::Init(render::cRenderContext* context, s32 width, s32 height, s32 depth)
        {
            m_context = context;
            m_atlas = m_context->CreateTexture(width, height, depth, render::sTexture::eType::TEXTURE_2D_ARRAY, render::sTexture::eFormat::RGBA8, nullptr);
            m_atlas->Slot = 0;
        }

        void mTexture::Free()
        {
            m_context->DeleteTexture(m_atlas);
            delete m_atlas;
        }

        sArea* mTexture::CreateTexture(const char* filename, const std::string& tag)
        {
            int width = 0, height = 0, channels = 0;
            unsigned char* data = nullptr;
            data = stbi_load(filename, &width, &height, &channels, 4);

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

            for (int layer = 0; layer < m_atlas->Depth; layer++)
            {
                for (int y = 0; y < m_atlas->Height; y++)
                {
                    for (int x = 0; x < m_atlas->Width; x++)
                    {
                        bool isIntersecting = false;

                        for (auto& area : m_textures)
                        {
                            if (area->IsNormalized == core::K_FALSE)
                            {
                                const glm::vec4 textureRect = glm::vec4(
                                    x, y, x + width, y + height
                                );
                                if ((area->Offset.z == layer &&
                                    area->Offset.x <= textureRect.z && area->Offset.x + area->Size.x >= textureRect.x &&
                                    area->Offset.y <= textureRect.w && area->Offset.y + area->Size.y >= textureRect.y) ||
                                    (x + width > m_atlas->Width || y + height > m_atlas->Height))
                                {
                                    isIntersecting = true;
                                    break;
                                }
                            }
                            else if (area->IsNormalized == core::K_TRUE)
                            {
                                const glm::vec4 textureRectNorm = glm::vec4(
                                    (float)x / (float)m_atlas->Width, (float)y / (float)m_atlas->Height,
                                    ((float)x + (float)width) / (float)m_atlas->Width, ((float)y + (float)height) / (float)m_atlas->Height
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
                            sArea* area = new sArea();
                            area->Tag = tag;
                            area->Offset = glm::vec3(x, y, layer);
                            area->Size = glm::vec2(width, height);

                            m_textures.push_back(area);

                            m_context->WriteTexture(m_atlas, area->Offset, area->Size, data);

                            free(data);

                            return area;
                        }
                    }
                }
            }

            free(data);

            return nullptr;
        }

        void mTexture::LoadAnimation(const std::vector<const char*>& filenames, const std::string& tag, std::vector<sArea*>& frames)
        {
            core::s32 i = 0;
            for (auto filename : filenames)
            {
                sArea* area = CreateTexture(filename, tag);
                if (area != nullptr)
                {
                    *area = CalculateNormalizedArea(*area);
                    i += 1;

                    frames.emplace_back(area);
                }
            }

            m_animations.emplace_back(frames);
        }

        void mTexture::RemoveTexture(const std::string& tag)
        {
            for (auto it = m_textures.begin(); it != m_textures.end(); it++)
            {
                if ((*it)->Tag == tag)
                {
                    m_textures.erase(it);
                    return;
                }
            }
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
            for (auto it = m_textures.begin(); it != m_textures.end();)
            {
                if ((*it)->Tag == tag)
                {
                    m_textures.erase(it);
                }
                else
                {
                    it++;
                }
            }

            for (auto it = m_animations.begin(); it != m_animations.end(); it++)
            {
                if ((*it)[0]->Tag == tag)
                {
                    m_animations.erase(it);
                    return;
                }
            }
        }

        sArea* mTexture::GetTexture(const std::string& tag)
        {
            for (auto& area : m_textures) {
                if (area->Tag == tag) {
                    return area;
                }
            }

            return nullptr;
        }

        std::vector<sArea*>* mTexture::GetAnimation(const std::string& tag)
        {
            for (auto& animation : m_animations) {
                for (auto& area : animation) {
                    if (area->Tag == tag) {
                        return &animation;
                    }
                }
            }

            return nullptr;
        }

        s32 mTexture::GetWidth()
        {
            return m_atlas->Width;
        }

        s32 mTexture::GetHeight()
        {
            return m_atlas->Height;
        }

        s32 mTexture::GetDepth()
        {
            return m_atlas->Depth;
        }

        sArea mTexture::CalculateNormalizedArea(const sArea& area)
        {
            sArea a;
            a.Tag = area.Tag;
            a.IsNormalized = K_TRUE;
            a.Offset.x = area.Offset.x / m_atlas->Width;
            a.Offset.y = area.Offset.y / m_atlas->Height;
            a.Offset.z = area.Offset.z;
            a.Size.x = area.Size.x / m_atlas->Width;
            a.Size.y = area.Size.y / m_atlas->Height;

            return a;
        }
    }
}