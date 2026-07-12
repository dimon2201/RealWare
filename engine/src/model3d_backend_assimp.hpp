// model3d_backend_assimp.hpp

#pragma once

#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <vector>
#include <unordered_map>
#include "model3d_backend.hpp"
#include "graphics_resource_backend.hpp"
#include "handles.hpp"

namespace triton
{
    struct SVertex;
    class cVector3;
    class XTextureSubsystem;
    class XMaterialSubsystem;
    struct SBone;

    struct SModel3DMaterialData final
    {
        types::s32 absoluteMaterialIndex = -1;
        std::string diffuseTextureFilePath = {};
        std::string normalTextureFilePath = {};
        std::string roughnessTextureFilePath = {};
        std::string metallicTextureFilePath = {};
        types::boolean bIsDiffuseEmbedded = types::K_FALSE;
        types::boolean bIsNormalEmbedded = types::K_FALSE;
        types::boolean bIsRoughnessEmbedded = types::K_FALSE;
        types::boolean bIsMetallicEmbedded = types::K_FALSE;
    };

    struct SBoneWeight final
    {
        types::usize boneIndex = 0;
        types::f32 weight = 0.0f;
    };

    // TODO: move this structs to separate file
    // ||||||||||||||||||||||||||||||||||||||||
    // VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
    struct SBonePositionKey
    {
        types::f32 time = 0.0f;
        cVector3 position = cVector3(0.0f);
    };

    struct SBoneRotationKey
    {
        types::f32 time = 0.0f;
        cQuaternion rotation = cQuaternion(0.0f, 0.0f, 0.0f, 0.0f);
    };

    struct SBoneScaleKey
    {
        types::f32 time = 0.0f;
        cVector3 scale = cVector3(0.0f);
    };

    struct SBoneAnimation
    {
        types::usize boneIndex = 0;
        std::vector<SBonePositionKey> positionKeys = {};
        std::vector<SBoneRotationKey> rotationKeys = {};
        std::vector<SBoneScaleKey> scaleKeys = {};
    };

    struct SAnimation
    {
        std::string name = "";
        types::f32 duration = 0.0f;
        types::f32 ticksPerSecond = 0.0f;
        std::vector<SBoneAnimation> bones = {};
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
        void CreateMaterials(const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, XMaterialSubsystem* materialSubsystem, const std::vector<SModel3DMaterialData>& materials, std::vector<HMaterial>& modelMaterials, const aiScene* scene);
        void SetAbsoluteMaterialIndices(SVertex*& vertexData, types::usize vertexCount, const std::vector<HMaterial>& modelMaterials);
        void DeallocateTempBitangentBuffer(cVector3* bitangents);
        
        void CreateBones(
            const aiScene* scene,
            SVertex* vertexData,
            types::usize vertexCount,
            std::unordered_map<std::string, types::usize>& boneIndices,
            std::vector<SBone>& bones,
            std::vector<std::vector<SBoneWeight>>& vertexWeights
        );

        void FinalizeBoneWeights(SVertex* vertexData, types::usize vertexCount, std::vector<std::vector<SBoneWeight>>& vertexWeights);
        
        void CreateBoneHierarchy(
            const aiNode* node,
            int parentBone,
            std::unordered_map<std::string, types::usize>& boneIndices,
            std::vector<SBone>& bones
        );

        void CreateAnimations(
            const aiScene* scene,
            const std::unordered_map<std::string, types::usize>& boneIndices,
            std::vector<SAnimation>& animations
        );
        
        std::optional<triton::HTexture> CreateTexture(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, types::boolean bIsEmbedded, const aiTexture* texture);
        std::optional<triton::HTexture> CreateTextureFromModelData(cTexture::eFormat dataFormat, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, const aiTexture* texture);
        std::optional<triton::HTexture> CreateTextureFromFile(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath);
        SModel3DBackendResource PrepareResult(const SVertex* vertexData, const types::u32* indexData, types::usize vertexCount, types::usize indexCount, const std::vector<HMaterial>& modelMaterials);
        cMatrix4 ConvertMatrix(const aiMatrix4x4& mat);
    };
}