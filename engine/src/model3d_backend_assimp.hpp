// model3d_backend_assimp.hpp

#pragma once

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <vector>
#include "model3d_backend.hpp"
#include "handles.hpp"

namespace triton
{
    struct SVertex;
    class cVector3;
    class XTextureSubsystem;
    class XMaterialSubsystem;

    struct SModel3DMaterialData final
    {
        types::s32 absoluteMaterialIndex = -1;
        std::string diffuseTextureFilePath = {};
        std::string normalTextureFilePath = {};
    };

    class XModel3DBackendAssimp final : public IModel3DBackend
    {
        TRITON_OBJECT(XModel3DBackendAssimp)

    public:
        explicit XModel3DBackendAssimp(cContext* context) : IModel3DBackend(context) {}
        ~XModel3DBackendAssimp() override = default;

        std::optional<SModel3DBackendResource> CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath) override final;
        void DestroyModel(SModel3DBackendResource& model) override final;

    private:
        void ImportScene(Assimp::Importer& importer, const aiScene*& scene, const std::string& filePath);
        void CountVerticesIndices(const aiScene* scene, types::usize& vertexCount, types::usize& indexCount, std::vector<types::usize>& indexOffsets);
        void AllocateVertexIndexBuffers(SVertex*& vertexData, types::u32*& indices, types::usize vertexCount, types::usize indexCount);
        void AllocateTempBitangentBuffer(cVector3*& bitangents, types::usize vertexCount);
        void ParseVertexData(const aiScene* scene, SVertex* vertexData, cVector3* bitangents);
        void CalculateHandedness(SVertex* vertexData, cVector3* bitangents, types::usize vertexCount);
        void ParseIndexData(const aiScene* scene, types::u32* indexData, const std::vector<types::usize>& indexOffsets);
        void ParseMaterialData(const aiScene* scene, std::vector<SModel3DMaterialData>& materials);
        void CreateMaterials(const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, XMaterialSubsystem* materialSubsystem, const std::vector<SModel3DMaterialData>& materials, std::vector<HMaterial>& modelMaterials);
        void SetAbsoluteMaterialIndices(SVertex*& vertexData, types::usize vertexCount, const std::vector<HMaterial>& modelMaterials);
        void DeallocateTempBitangentBuffer(cVector3* bitangents);
        SModel3DBackendResource PrepareResult(const SVertex* vertexData, const types::u32* indexData, types::usize vertexCount, types::usize indexCount, const std::vector<HMaterial>& modelMaterials);
    };
}