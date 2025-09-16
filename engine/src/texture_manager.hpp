#pragma once

#include <string>
#include <vector>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "id_vec.hpp"
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

        struct sTextureAtlasTexture : public utils::sIdVecObject
        {
            sTextureAtlasTexture(const types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size);
            ~sTextureAtlasTexture() = default;

            types::boolean IsNormalized = types::K_FALSE;
            glm::vec3 Offset = glm::vec3(0.0f);
            glm::vec2 Size = glm::vec2(0.0f);
        };

        struct sTextureAtlasTextureGPU
        {
            glm::vec4 TextureInfo = glm::vec4(0.0f);
            types::f32 TextureLayerInfo = 0.0f;
        };

        class mTexture
        {
        public:
            explicit mTexture(const app::cApplication* const app, const cRenderContext* const context);
            ~mTexture();

            sTextureAtlasTexture* AddTexture(const std::string& id, const glm::vec2& size, const types::usize channels, const types::u8* data);
            sTextureAtlasTexture* AddTexture(const std::string& id, const std::string& filename);
            sTextureAtlasTexture* FindTexture(const std::string& id);
            void DeleteTexture(const std::string& id);

            sTextureAtlasTexture CalculateNormalizedArea(const sTextureAtlasTexture& area);

            sTexture* GetAtlas();
            inline types::usize GetWidth() const;
            inline types::usize GetHeight() const;
            inline types::usize GetDepth() const;

        protected:
            app::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sTexture* _atlas = nullptr;
            utils::cIdVec<sTextureAtlasTexture> _textures;
        };
    }
}