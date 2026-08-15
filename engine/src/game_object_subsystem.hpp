// game_object_subsystem.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "game_object.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance_data.hpp"
#include "model3d_data.hpp"
#include "batch_data.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;
    template <typename TValue>
    class XLinearArray;
    class cVector3;
    struct SBatchInstance;
    class XGameObjectPool;

    class XGameObjectSubsystem : public ISubsys
    {
        TRITON_OBJECT(XGameObjectSubsystem)

        XGameObjectPool* _pool = nullptr;

    public:
        explicit XGameObjectSubsystem(cContext* context);
        ~XGameObjectSubsystem() override;

        std::optional<XGameObject::THandle> Create(
            const std::string& name,
            const SBatchData::THandle& batchHandle
        );

        std::optional<XGameObject::THandle> Create(
            const std::string& name,
            const SModel3DData& model,
            const SBatchData::THandle& batchHandle
        );

        void Destroy(const XGameObject::THandle& gameObject);

        void Init() override;
        void Free() override;
        void Update() override;

        XGameObjectPool* GetPool() const
        {
            return _pool;
        }
    };
}