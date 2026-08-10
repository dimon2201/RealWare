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

        std::optional<HStaticRenderInstance> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            EGraphicsBufferFormat format,
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount,
            const std::optional<HBatch>& existingBatch = std::nullopt,
            const std::optional<HMaterial>& existingMaterial = std::nullopt
        );
        
        std::optional<HStaticRenderInstance> SetRenderableStatic(
            const SGameObjectData::THandle& gameObject,
            const HModel3D& model,
            const std::optional<HBatch>& existingBatch = std::nullopt,
            const std::optional<HMaterial>& existingMaterial = std::nullopt
        );

        std::optional<HDynamicRenderInstance> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            EGraphicsBufferFormat format,
            const types::u8* vertexBytes,
            types::usize vertexBytesCount,
            const types::u8* indexBytes,
            types::usize indexBytesCount,
            const std::optional<HBatch>& existingBatch = std::nullopt,
            const std::optional<HMaterial>& existingMaterial = std::nullopt
        );

        std::optional<HDynamicRenderInstance> SetRenderableDynamic(
            const SGameObjectData::THandle& gameObject,
            const HModel3D& model,
            const std::optional<HBatch>& existingBatch = std::nullopt,
            const std::optional<HMaterial>& existingMaterial = std::nullopt
        );
    };
}