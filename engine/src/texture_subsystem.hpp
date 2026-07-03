// texture_subsystem.hpp

#pragma once

#include <string>
#include <vector>
#include <optional>
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

    struct sTextureAtlasTextureGPU
    {
        glm::vec4 _textureInfo = glm::vec4(0.0f);
        types::f32 _textureLayerInfo = 0.0f;
    };

    enum class ETextureFormat
    {
        NONE,
        PNG,
        DDS
    };

    class XTextureSubsystem : public ISubsystem<HTexture, STexture, XLinearArray<STexture>>
    {
        TRITON_OBJECT(XTextureSubsystem)

        cTexture* _atlas = nullptr;

    public:
        explicit XTextureSubsystem(cContext* context, const cVector3& size);
        ~XTextureSubsystem() override;

        HTexture CreateTexture(const std::string& filePath);
        void Init() override {}
        void Free() override {}
        void Update() override {}

        inline cTexture* GetAtlas() const
        {
            return _atlas;
        }

    private:
        std::optional<STexture> CreateTexture(cTexture::eFormat format, const cVector2& size, const types::u8* data);
        std::optional<STexture> CreateTextureFromFile(const std::string& filePath);
        types::boolean IsOverlapping(const STexture& candidateTexture, const STexture& atlasTexture);
    };
}