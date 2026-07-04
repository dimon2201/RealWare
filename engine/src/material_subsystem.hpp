// material_subsystem.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "game_object.hpp"
#include "handles.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance.hpp"
#include "material.hpp"
#include "types.hpp"

namespace triton
{
    class XMaterialUploader;

    class XMaterialSubsystem : public ISubsystem<HMaterial, SMaterial, XLinearArray<SMaterial>>
    {
        TRITON_OBJECT(XMaterialSubsystem)

        XMaterialUploader* _uploader = nullptr;

    public:
        explicit XMaterialSubsystem(cContext* context);
        ~XMaterialSubsystem() override;

        HMaterial CreateMaterial(
            const cVector4& diffuseColor,
            const HTexture& diffuseTexture,
            const HTexture& normalTexture,
            const HTexture& roughnessTexture,
            const HTexture& metallicTexture
        );
        void Set(types::usize materialIndex, const SMaterial& materialData);
        void Init() override;
        void Free() override;
        void Update() override;

    private:
        void MarkDirty();
    };
}