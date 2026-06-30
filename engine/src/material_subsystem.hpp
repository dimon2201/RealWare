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

    struct SMaterialTextureLayout
    {
        types::u32 atlasLayer = 0;
        types::u32 _pad = 0;
        cVector2 atlasNormOffset = cVector2(0.0f);
        cVector2 atlasNormSize = cVector2(0.0f);
    };

    struct SMaterialLayout
    {
        SMaterialTextureLayout diffuse;
        types::u32 _pad[2];
        cVector4 diffuseColor = cVector4(0.0f);
    };

    class XMaterialSubsystem : public ISubsystem<HMaterial, SMaterial, XLinearArray<SGameObject>>
    {
        TRITON_OBJECT(XMaterialSubsystem)
        TRITON_SUBSYSTEM

        XMaterialUploader* _uploader = nullptr;

    public:
        inline HMaterial CreateMaterial(const std::string& diffuseTextureFilePath)
        {
            HMaterial material = Create();
            Get(material).name = name;

            return gameObject;
        }

        void Init() override;
        void Free() override;
        void Update() override;

    private:
        void MarkDirty();
        void WriteDirty();
    };
}