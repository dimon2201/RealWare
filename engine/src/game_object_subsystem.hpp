// game_object_subsystem.hpp

#pragma once

#include <optional>
#include "object.hpp"
#include "DELETE_THIS_FILE_ASAP.hpp"
#include "game_object_data.hpp"
#include "graphics_buffer_formats.hpp"
#include "render_instance_data.hpp"
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

        std::optional<SGameObjectData::THandle> Create(const std::string& name);

        void Init() override;
        void Free() override;
        void Update() override;

        std::optional<SStaticRenderInstanceData::THandle> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            EVertexBufferFormat format,
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount,
            const std::optional<SBatchData::THandle>& existingBatch = std::nullopt,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );
        
        std::optional<SStaticRenderInstanceData::THandle> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            const SModel3DData::THandle& model,
            const std::optional<SBatchData::THandle>& existingBatch = std::nullopt,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );

        std::optional<SDynamicRenderInstanceData::THandle> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            EVertexBufferFormat format,
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount,
            const std::optional<SBatchData::THandle>& existingBatch = std::nullopt,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );

        std::optional<SDynamicRenderInstanceData::THandle> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            const SModel3DData::THandle& model,
            const std::optional<SBatchData::THandle>& existingBatch = std::nullopt,
            const std::optional<SMaterialData::THandle>& existingMaterial = std::nullopt
        );
    };
}