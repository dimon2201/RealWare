// model3d_backend.hpp

#pragma once

#include <string>
#include <optional>
#include "backend.hpp"
#include "object.hpp"
#include "model3d_formats.hpp"
#include "vertex.hpp"
#include "material.hpp"
#include "skeleton.hpp"
#include "animation.hpp"
#include "vertex_buffer_format.hpp"

namespace triton
{
    struct SVertex;

    struct SModel3DData
    {
        EVertexBufferFormat vertexDataFormat = EVertexBufferFormat::Unknown;
        const SRigidVertexGPULayout* rigidVertexData = nullptr;
        const SSkinnedVertexGPULayout* skinnedVertexData = nullptr;
        const types::u32* indexData = nullptr;
        types::usize vertexCount = 0;
        types::usize indexCount = 0;
        std::vector<XMaterial::THandle> materials = {};
        XSkeleton::THandle skeleton = {};
        std::vector<XAnimation::THandle> animations = {};
    };

    class IModel3DBackend : public iBackend
    {
        TRITON_CLASS_NAME(IModel3DBackend)

    public:
        explicit IModel3DBackend(cContext* context) : iBackend(context) {}
        ~IModel3DBackend() override = default;

        virtual std::optional<SModel3DData> CreateModel(
            const std::string& modelFolderPath,
            const std::string& modelLocalPath,
            EVertexBufferFormat vertexDataFormat
        ) = 0;

        virtual std::optional<SModel3DData> CreateModel(
            const types::u8* byteData,
            types::usize byteSize,
            EVertexBufferFormat vertexDataFormat
        ) = 0;

        virtual void DestroyModel(SModel3DData& model) = 0;
    };
}