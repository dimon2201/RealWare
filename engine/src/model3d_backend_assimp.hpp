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
    class XSkeletonSubsystem;
    class XAnimationSubsystem;
    struct SBone;
    struct SAnimation;
    struct SSkeleton;

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

    class XModel3DBackendAssimp final : public IModel3DBackend
    {
        TRITON_OBJECT(XModel3DBackendAssimp)

    public:
        explicit XModel3DBackendAssimp(cContext* context) : IModel3DBackend(context) {}
        ~XModel3DBackendAssimp() override = default;

        std::optional<SModel3DData> CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath) override final;
        void DestroyModel(SModel3DData& model) override final;

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
        
        types::boolean AccumulateRootTransform(
            const aiNode* node,
            aiNode*& boneRootNode,
            const aiMatrix4x4& parentRootTransform,
            aiMatrix4x4& accumulatedRootTransform,
            std::unordered_map<std::string, types::usize>& boneIndices
        );

        void CreateBoneHierarchy(
            const aiNode* node,
            types::s32 parentBone,
            std::unordered_map<std::string, types::usize>& boneIndices,
            std::vector<SBone>& bones
        );

        void CreateSkeleton(
            HSkeleton& modelSkeleton,
            const std::vector<SBone>& bones,
            XSkeletonSubsystem* skeletonSubsystem,
            const aiMatrix4x4& accumulatedRootTransform
        );

        void CreateAnimations(
            const aiScene* scene,
            const std::unordered_map<std::string, types::usize>& boneIndices,
            XAnimationSubsystem* animationSubsystem,
            HSkeleton modelSkeleton,
            std::vector<HAnimation>& modelAnimations
        );
        
        std::optional<triton::HTexture> CreateTexture(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, types::boolean bIsEmbedded, const aiTexture* texture);
        std::optional<triton::HTexture> CreateTextureFromModelData(cTexture::eFormat dataFormat, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, const aiTexture* texture);
        std::optional<triton::HTexture> CreateTextureFromFile(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath);
        SModel3DData PrepareResult(
            const SVertex* vertexData,
            const types::u32* indexData,
            types::usize vertexCount,
            types::usize indexCount,
            const std::vector<HMaterial>& modelMaterials,
            HSkeleton modelSkeleton,
            const std::vector<HAnimation>& modelAnimations
        );
        cMatrix4 ConvertMatrix(const aiMatrix4x4& mat);
    };
}