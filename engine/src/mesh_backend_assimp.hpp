// mesh_backend_assimp.hpp

#pragma once

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <vector>
#include "mesh_backend.hpp"

namespace triton
{
    struct SVertex;
    class cVector3;

    class CMeshBackendAssimp final : public IMeshBackend
    {
        TRITON_OBJECT(CMeshBackendAssimp)

    public:
        explicit CMeshBackendAssimp(cContext* context) : IMeshBackend(context) {}
        ~CMeshBackendAssimp() override = default;

        std::optional<SMeshBackendResource> CreateMesh(const std::string& filePath) override final;
        void DestroyMesh(SMeshBackendResource& sound) override final;

    private:
        void ImportScene(Assimp::Importer& importer, const aiScene*& scene, const std::string& filePath);
        void CountVerticesIndices(const aiScene* scene, types::usize& vertexCount, types::usize& indexCount, std::vector<types::usize>& indexOffsets);
        void AllocateVertexIndexBuffers(SVertex*& vertexData, types::u32*& indices, types::usize vertexCount, types::usize indexCount);
        void AllocateTempBitangentBuffer(cVector3*& bitangents, types::usize vertexCount);
        void ParseVertexData(const aiScene* scene, SVertex* vertexData, cVector3* bitangents);
        void CalculateHandedness(SVertex* vertexData, cVector3* bitangents, types::usize vertexCount);
        void ParseIndexData(const aiScene* scene, types::u32* indexData, const std::vector<types::usize>& indexOffsets);
        void DeallocateTempBitangentBuffer(cVector3* bitangents);
        SMeshBackendResource PrepareResult(const SVertex* vertexData, const types::u32* indexData, types::usize vertexCount, types::usize indexCount);
    };
}