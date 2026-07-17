// game_object_subsystem.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "game_object.hpp"
#include "handles.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance.hpp"
#include "types.hpp"

namespace triton
{
    template <typename TValue>
    class XLinearArray;
    class cVector3;
    struct SBatchInstance;

    class XGameObjectSubsystem : public ISubsystem<HGameObject, SGameObjectData, XLinearArray<SGameObjectData>>
    {
        TRITON_OBJECT(XGameObjectSubsystem)
        TRITON_SUBSYSTEM

    public:
        inline HGameObject CreateGameObject(const std::string& name)
        {
            HGameObject gameObject = Create();
            Get(gameObject).name = name;
            
            return gameObject;
        }

        void Init() override;
        void Free() override;
        void Update() override;

        HRenderInstance SetRenderable(
            const HGameObject& gameObject,
            ERenderInstanceMotionType usage,
            EGraphicsBufferFormat format,
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount,
            const std::optional<HBatch>& existingBatch = std::nullopt,
            const std::optional<HMaterial>& existingMaterial = std::nullopt
        );
        
        void AddRenderable(
            const HGameObject& gameObject,
            ERenderInstanceMotionType usage,
            const HModel3D& model
        );
        
        void SetMaterial(
            const HGameObject& gameObject,
            const HMaterial& material
        );
    };
}