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

        struct cTextureAtlasTexture : public utils::cIdVecObject
        {
        public:
            explicit cTextureAtlasTexture(const types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size);
            ~cTextureAtlasTexture() = default;

            inline types::boolean IsNormalized() const { return _isNormalized; }
            inline const glm::vec3& GetOffset() const { return _offset; }
            inline const glm::vec2& GetSize() const { return _size; }

        private:
            types::boolean _isNormalized = types::K_FALSE;
            glm::vec3 _offset = glm::vec3(0.0f);
            glm::vec2 _size = glm::vec2(0.0f);
        };

        class mTexture
        {
        public:
            explicit mTexture(const app::cApplication* const app, const cRenderContext* const context);
            ~mTexture();

            cTextureAtlasTexture* AddTexture(const std::string& id, const std::string& filename);
            cTextureAtlasTexture* FindTexture(const std::string& id);
            void DeleteTexture(const std::string& id);

            cTextureAtlasTexture CalculateNormalizedArea(const cTextureAtlasTexture& area);

            cTextureAtlasTexture* GetTexture(const std::string& id);
            sTexture* GetAtlas();
            inline types::usize GetWidth() const;
            inline types::usize GetHeight() const;
            inline types::usize GetDepth() const;

        protected:
            app::cApplication* _app = nullptr;
            cRenderContext* _context = nullptr;
            sTexture* _atlas = nullptr;
            utils::cIdVec<cTextureAtlasTexture> _textures;
        };
    }
}