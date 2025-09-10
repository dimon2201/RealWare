#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace core
    {
        class cApplication;
        class cScene;
    }

    namespace render
    {
        class cRenderContext;
        struct sTexture;

        struct sTextureAtlasTexture
        {
            std::string Tag = "";
            core::boolean IsNormalized = core::K_FALSE;
            glm::vec3 Offset = glm::vec3(0.0f);
            glm::vec2 Size = glm::vec2(0.0f);
        };

        class mTexture
        {
        public:
            explicit mTexture(const core::cApplication* const app, const cRenderContext* const context);
            ~mTexture();

            sTextureAtlasTexture* AddTexture(const std::string& filename, const std::string& tag);
            void DeleteTexture(const std::string& tag);
            //void LoadAnimation(const std::vector<const std::string&>& filenames, const std::string& tag, std::vector<sTextureAtlasTexture*>& frames);
            //void PlayAnimation(entity object, cScene* scene, float speed);
            //void IncrementAnimationFrame(entity object, cScene* scene);
            //void RemoveAnimation(const std::string& tag);

            sTextureAtlasTexture CalculateNormalizedArea(const sTextureAtlasTexture& area);

            sTextureAtlasTexture* GetTexture(const std::string& tag);
            std::vector<sTextureAtlasTexture*>* GetAnimation(const std::string& tag);
            sTexture* GetAtlas();
            inline core::usize GetWidth() const;
            inline core::usize GetHeight() const;
            inline core::usize GetDepth() const;

        protected:
            core::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sTexture* _atlas = nullptr;
            std::vector<sTextureAtlasTexture*> _textures = {};
            std::vector<std::vector<sTextureAtlasTexture*>> _animations = {};

        };
    }
}