// game_object_subsystem.hpp

#pragma once

#include "object.hpp"
#include "subsystem.hpp"
#include "game_object.hpp"
#include "handles.hpp"

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
        void Init() override;
        void Free() override;
        void Update() override;
        void AddRenderable(const HGameObject& gameObject);
        void SetWorldPosition(const HGameObject& gameObject, const cVector3& worldPosition);

    private:
        void AddDirty(const SBatchInstance& renderable);
        void WriteDirty();
    };
}