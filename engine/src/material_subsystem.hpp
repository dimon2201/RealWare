// material_subsystem.hpp

#pragma once

#include "object.hpp"
#include "game_object_data.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance_data.hpp"
#include "material_data.hpp"
#include "types.hpp"

#include "DELETE_THIS_FILE_ASAP.hpp"

namespace triton
{
    class XMaterialPool;

    class XMaterialSubsystem : public ISubsys
    {
        TRITON_OBJECT(XMaterialSubsystem)

        XMaterialPool* _pool = nullptr;

    public:
        explicit XMaterialSubsystem(cContext* context);
        ~XMaterialSubsystem() override;

        std::optional<SMaterialData::THandle> Create(
            const cVector4& diffuseColor,
            const HTexture& diffuseTexture,
            const HTexture& normalTexture,
            const HTexture& roughnessTexture,
            const HTexture& metallicTexture
        );

        void Destroy(const SMaterialData::THandle& material);

        void Init() override {}

        void Free() override {}

        void Update() override;

        inline XMaterialPool* GetPool() const
        {
            return _pool;
        }
    };
}