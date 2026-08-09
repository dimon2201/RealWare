// material_subsystem.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "game_object.hpp"
#include "handles.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance_data.hpp"
#include "material.hpp"
#include "uploader.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
    class XMaterialUploader;

    class XMaterialSubsystem : public ISubsys,
                               public CUploader<SMaterial, HMaterial, XLinearArray<SMaterial>, SGPUMaterialLayout>
    {
        TRITON_OBJECT(XMaterialSubsystem)

        cBuffer* _materialGPUBuffer = nullptr;

    public:
        explicit XMaterialSubsystem(cContext* context);
        ~XMaterialSubsystem() override = default;

        HMaterial CreateMaterial(
            const cVector4& diffuseColor,
            const HTexture& diffuseTexture,
            const HTexture& normalTexture,
            const HTexture& roughnessTexture,
            const HTexture& metallicTexture
        );

        void DestroyMaterial(const HMaterial& material);

        void Set(types::usize materialIndex, const SMaterial& materialData);

        void Init() override;

        void Free() override;

        void Update() override;

        inline cBuffer& GetMaterialGPUBuffer() const
        {
            return *_materialGPUBuffer;
        }

    private:
        SGPUMaterialLayout ConvertToGPULayout(const HMaterial& material);
    };
}