#pragma once

#include <string>
#include <vector>

#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace render
    {
        struct sTexture;
        class cRenderContext;
    }

    namespace core
    {
        class cApplication;
        class cScene;
        class mTexture;

        using entity = u64;

        struct sArea
        {
            std::string Tag = "";
            boolean IsNormalized = K_FALSE;
            glm::vec3 Offset;
            glm::vec2 Size;
        };

        class mTexture
        {

        public:
            mTexture(cApplication* app, render::cRenderContext* context, s32 width, s32 height, s32 depth);
            ~mTexture();

            sArea* CreateTexture(const std::string& filename, const std::string& tag);
            void RemoveTexture(const std::string& tag);
            void LoadAnimation(const std::vector<const char*>& filenames, const std::string& tag, std::vector<sArea*>& frames);
            void PlayAnimation(entity object, cScene* scene, float speed);
            void IncrementAnimationFrame(entity object, cScene* scene);
            void RemoveAnimation(const std::string& tag);

            sArea* GetTexture(const std::string& tag);
            std::vector<sArea*>* GetAnimation(const std::string& tag);
            inline render::sTexture* GetAtlas() { return m_atlas; }

            sArea CalculateNormalizedArea(const sArea& area);

            s32 GetWidth();
            s32 GetHeight();
            s32 GetDepth();

        protected:
            cApplication* m_app;
            render::cRenderContext* m_context;
            render::sTexture* m_atlas;
            std::vector<sArea*> m_textures;
            std::vector<std::vector<sArea*>> m_animations;

        };
    }
}