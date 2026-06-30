// texture_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include "object.hpp"
#include "math.hpp"
#include "linear_array.hpp"
#include "subsystem.hpp"
#include "handles.hpp"
#include "graphics_resource_backend.hpp"
#include "texture.hpp"
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

    class XTextureSubsystem : public ISubsystem<HTexture, STexture, XLinearArray<STexture>>
    {
        TRITON_OBJECT(XTextureSubsystem)

        cTexture* _atlas = nullptr;

    public:
        explicit XTextureSubsystem(cContext* context, const cVector3& size);
        ~XTextureSubsystem() override;

        inline HTexture CreateTexture(const std::string& filePath)
        {
            HTexture texture = Create();

            return texture;
        }

        void Init() override {}
        void Free() override {}
        void Update() override {}

    private:
        HTexture CreateTexture(cTexture::eFormat format, const cVector2& size, const types::u8* data);
        HTexture CreateTexture(const std::string& filePath);
        void DestroyTexture(const HTexture& texture);
    };
}