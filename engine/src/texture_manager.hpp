// texture_manager.hpp

#pragma once

#include <string>
#include <vector>
#include "object.hpp"
#include "math.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    class iGraphicsBackend;
    class cApplication;
    class cTexture;

    class cTextureAtlasTexture : public iObject
    {
        TRITON_OBJECT(cTextureAtlasTexture)

        types::boolean _isNormalized = types::K_FALSE;
        glm::vec3 _offset = glm::vec3(0.0f);
        glm::vec2 _size = glm::vec2(0.0f);

    public:
        cTextureAtlasTexture(cContext* context, types::boolean isNormalized, const glm::vec3& offset, const glm::vec2& size, cTexture* atlas = nullptr);
        ~cTextureAtlasTexture() = default;

        inline types::boolean IsNormalized() const { return _isNormalized; }
        inline const glm::vec3& GetOffset() const { return _offset; }
        inline const glm::vec2& GetSize() const { return _size; }
    };

    struct sTextureAtlasTextureGPU
    {
        glm::vec4 _textureInfo = glm::vec4(0.0f);
        types::f32 _textureLayerInfo = 0.0f;
    };

    class cTextureAtlas : public iObject
    {
        TRITON_OBJECT(cTextureAtlas)

    protected:
        iGraphicsBackend* _gfx = nullptr;
        cTexture* _atlas = nullptr;

    public:
        explicit cTextureAtlas(cContext* context);
        virtual ~cTextureAtlas() override final = default;

        void Initialize(const cVector3& size);
        void Shutdown();

        // TODO: New implementation of texture creation
        //cTextureAtlasTexture* CreateTexture(const std::string& id, const glm::vec2& size, types::usize channels, const types::u8* data);
        //cTextureAtlasTexture* CreateTexture(const std::string& id, const std::string& filename);
        //cTextureAtlasTexture* FindTexture(const std::string& id);
        //void DestroyTexture(const std::string& id);

        cTexture* GetAtlas() const;
        types::usize GetWidth() const;
        types::usize GetHeight() const;
        types::usize GetDepth() const;
    };
}