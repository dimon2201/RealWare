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

    struct SDirtyBufferItem
    {
        SBatchInstance renderable;
        SRenderInstance renderInstance;
    };

    class XGameObjectSubsystem : public ISubsystem<HGameObject, SGameObject, XLinearArray<SGameObject>>
    {
        TRITON_OBJECT(XGameObjectSubsystem)
        TRITON_SUBSYSTEM

        std::vector<SDirtyBufferItem> _dirtyBuffer = {};

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
        void AddRenderable(const HGameObject& gameObject, SRenderInstance::EUsage usage, EGraphicsBufferFormat format, const types::u8* vertexBytes, types::usize vertexBytesCount, const types::u8* indexBytes, types::usize indexBytesCount);
        
        void AddRenderable(
            const HGameObject& gameObject,
            SRenderInstance::EUsage usage,
            const HModel3D& model
        );
        
        void SetWorldPosition(const HGameObject& gameObject, const cVector3& worldPosition);
        void SetWorldRotation(const HGameObject& gameObject, const cVector3& worldRotation);
        void SetMaterial(const HGameObject& gameObject, const HMaterial& material);

    private:
        void AddDirty(const SBatchInstance& batchInstance, const SRenderInstance& renderInstance, const cVector3& worldPosition, const cVector3& worldRotation);
        void WriteDirty();
    };
}