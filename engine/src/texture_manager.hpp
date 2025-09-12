#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        class cRenderContext;
        struct sTexture;

        struct sTextureAtlasTexture
        {
            std::string ID = "";
            types::boolean IsNormalized = types::K_FALSE;
            glm::vec3 Offset = glm::vec3(0.0f);
            glm::vec2 Size = glm::vec2(0.0f);
        };

        class mTexture
        {
        public:
            explicit mTexture(const app::cApplication* const app, const cRenderContext* const context);
            ~mTexture();

            sTextureAtlasTexture* AddTexture(const std::string& id, const std::string& filename);
            void DeleteTexture(const std::string& id);
            //void LoadAnimation(const std::vector<const std::string&>& filenames, const std::string& tag, std::vector<sTextureAtlasTexture*>& frames);
            //void PlayAnimation(entity object, cScene* scene, float speed);
            //void IncrementAnimationFrame(entity object, cScene* scene);
            //void RemoveAnimation(const std::string& tag);

            sTextureAtlasTexture CalculateNormalizedArea(const sTextureAtlasTexture& area);

            sTextureAtlasTexture* GetTexture(const std::string& id);
            std::vector<sTextureAtlasTexture*>* GetAnimation(const std::string& id);
            sTexture* GetAtlas();
            inline types::usize GetWidth() const;
            inline types::usize GetHeight() const;
            inline types::usize GetDepth() const;

        protected:
            app::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sTexture* _atlas = nullptr;
            std::vector<sTextureAtlasTexture*> _textures = {};
            std::vector<std::vector<sTextureAtlasTexture*>> _animations = {};
        };
    }
}