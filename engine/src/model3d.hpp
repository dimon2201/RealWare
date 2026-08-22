// model3d.hpp

#pragma once

#include <vector>
#include <filesystem>
#include "../../tools/tasset/src/tasset/tasset.hpp"
#include "object.hpp"
#include "vertex.hpp"
#include "animation.hpp"
#include "material.hpp"
#include "model3d_backend.hpp"
#include "types.hpp"

namespace triton
{
    class cContext;

    enum class EModel3DFileType
    {
        Unknown,
        Raw,
        Asset
    };

	class XModel3D : public iObject
	{
        TRITON_CLASS_NAME(XModel3D)

        SModel3DData            _data;
        asset::CModel3DAsset*   _asset = nullptr;
        EVertexBufferFormat     _vertexDataFormat = EVertexBufferFormat::Unknown;
        EModel3DFileType        _fileType = EModel3DFileType::Unknown;

    public:
        explicit XModel3D(cContext* context, types::s32 poolIndex) : iObject(context, poolIndex) {}

        explicit XModel3D(
            cContext* context,
            types::s32 poolIndex,
            EVertexBufferFormat vertexDataFormat,
            EModel3DFileType fileType,
            const std::filesystem::path& filePath
        );

        ~XModel3D() override;

        inline const SRigidVertexGPULayout* GetRigidVertices() const { return _data.rigidVertexData; }

        inline const SSkinnedVertexGPULayout* GetSkinnedVertices() const { return _data.skinnedVertexData; }

        inline const types::u32* GetIndices() const { return _data.indexData; }

        inline types::usize GetVertexCount() const { return _data.vertexCount; }

        inline types::usize GetIndexCount() const { return _data.indexCount; }

        inline XMaterial::THandle GetMaterial() const
        {
            return _data.materials.size() > 0 ? _data.materials.at(0) : XMaterial::THandle();
        }

        struct THandle : public SHandle {};

        struct TGPULayout {};
	};
}