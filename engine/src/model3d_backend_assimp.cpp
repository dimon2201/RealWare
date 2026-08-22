// model3d_backend_assimp.cpp

#include <filesystem>
#include "model3d_backend_assimp.hpp"
#include "model3d.hpp"
#include "context.hpp"
#include "math.hpp"
#include "skeleton_bone.hpp"
#include "animation.hpp"
#include "material_pool.hpp"
#include "texture_atlas.hpp"
#include "atlas_texture_pool.hpp"
#include "object_allocator.hpp"
#include "skeleton_pool.hpp"
#include "animation_pool.hpp"

using namespace types;

std::optional<triton::SModel3DData> triton::XModel3DBackendAssimp::CreateModel(
    const std::string& modelFolderPath,
    const std::string& modelLocalPath,
    EVertexBufferFormat vertexDataFormat
)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ImportScene(
        importer,
        scene,
        modelFolderPath + "/" + modelLocalPath
    );

    return ParseImportedScene(
        _context,
        importer,
        scene,
        vertexDataFormat,
        modelFolderPath
    );
}

std::optional<triton::SModel3DData> triton::XModel3DBackendAssimp::CreateModel(
    const types::u8* byteData,
    const types::usize byteSize,
    EVertexBufferFormat vertexDataFormat
)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ImportScene(
        importer,
        scene,
        byteData,
        byteSize
    );

    return ParseImportedScene(
        _context,
        importer,
        scene,
        vertexDataFormat
    );
}

std::optional<triton::SModel3DData> triton::XModel3DBackendAssimp::ParseImportedScene(
    cContext* context,
    const Assimp::Importer& importer,
    const aiScene* scene,
    EVertexBufferFormat vertexDataFormat,
    const std::string& modelFolderPath
)
{
    if (!scene)
        return std::nullopt;

    usize vertexCount = 0;
    usize indexCount = 0;
    std::vector<usize> indexOffsets = {};
    SRigidVertexGPULayout* rigidVertexData = nullptr;
    SSkinnedVertexGPULayout* skinnedVertexData = nullptr;
    u32* indexData = nullptr;
    cVector3* bitangents = nullptr;
    std::vector<SModel3DMaterialData> materials = {};
    std::vector<XMaterial::THandle> modelMaterials = {};
    std::unordered_map<std::string, types::usize> boneIndices = {};
    std::vector<SSkeletonBone> bones = {};
    std::vector<std::vector<SBoneWeight>> vertexWeights;
    XSkeleton::THandle modelSkeleton = {};
    usize boneOffset = 0;
    std::vector<XAnimation::THandle> modelAnimations = {};
    aiNode* boneRootNode = nullptr;
    aiMatrix4x4 parentRootTransform = aiMatrix4x4();
    aiMatrix4x4 accumulatedRootTransform = aiMatrix4x4();

    CountVerticesIndices(scene, vertexCount, indexCount, indexOffsets);

    AllocateVertexIndexBuffers(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        indexData,
        vertexCount,
        indexCount
    );
    AllocateTempBitangentBuffer(bitangents, vertexCount);
    ParseVertexData(
        scene,
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        bitangents
    );
    CalculateHandedness(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        bitangents,
        vertexCount
    );
    DeallocateTempBitangentBuffer(bitangents);

    // TODO: encapsulate "is_directory()" check to proper file system backend
    if (std::filesystem::is_directory(modelFolderPath))
    {
        ParseMaterialData(scene, materials);
        CreateMaterials(
            context,
            modelFolderPath,
            context->GetSubsystem<CTextureAtlas>(),
            materials,
            modelMaterials,
            scene
        );
    }

    ParseIndexData(scene, indexData, indexOffsets);

    if (vertexDataFormat == EVertexBufferFormat::Skinned_80)
    {
        CreateBones(scene, skinnedVertexData, vertexCount, boneIndices, bones, vertexWeights);
        FinalizeBoneWeights(context, skinnedVertexData, vertexCount, vertexWeights);
        AccumulateRootTransform(
            scene->mRootNode,
            boneRootNode,
            parentRootTransform,
            accumulatedRootTransform,
            boneIndices
        );
        CreateBoneHierarchy(scene->mRootNode, -1, boneIndices, bones);
        CreateSkeleton(
            context,
            modelSkeleton,
            bones,
            accumulatedRootTransform
        );
        CreateAnimations(
            context,
            scene,
            boneIndices,
            modelSkeleton,
            modelAnimations
        );
    }

    return PrepareResult(
        vertexDataFormat,
        rigidVertexData,
        skinnedVertexData,
        indexData,
        vertexCount,
        indexCount,
        modelMaterials,
        modelSkeleton,
        modelAnimations
    );
}

void triton::XModel3DBackendAssimp::DestroyModel(SModel3DData& model)
{
    model.vertexCount = 0;
    model.indexCount = 0;
    if (model.indexData)
        CObjectAllocator::Deallocate((void*)model.indexData);
    if (model.skinnedVertexData)
        CObjectAllocator::Deallocate((void*)model.skinnedVertexData);
    if (model.rigidVertexData)
        CObjectAllocator::Deallocate((void*)model.rigidVertexData);
}

void triton::XModel3DBackendAssimp::ImportScene(
    Assimp::Importer& importer,
    const aiScene*& scene,
    const std::string& filePath
)
{
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    scene = importer.ReadFile(
        filePath.c_str(),
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );
    if (!scene)
        Print("Error: can't load 3d model from file '" + filePath + "'");
}

void triton::XModel3DBackendAssimp::ImportScene(
    Assimp::Importer& importer,
    const aiScene*& scene,
    const types::u8* byteData,
    types::usize byteSize
)
{
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    scene = importer.ReadFileFromMemory(
        byteData,
        byteSize,
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );
    if (!scene)
        Print("Error: can't load 3d model from byte data");
}

void triton::XModel3DBackendAssimp::CountVerticesIndices(const aiScene* scene, usize& vertexCount, usize& indexCount, std::vector<usize>& indexOffsets)
{
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        indexOffsets.push_back(vertexCount);
        vertexCount += scene->mMeshes[meshIndex]->mNumVertices;
    }
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
        if (scene->mMeshes[meshIndex]->HasFaces())
            for (usize faceIndex = 0; faceIndex < scene->mMeshes[meshIndex]->mNumFaces; faceIndex++)
                indexCount += scene->mMeshes[meshIndex]->mFaces[faceIndex].mNumIndices;
}

void triton::XModel3DBackendAssimp::AllocateVertexIndexBuffers(
    EVertexBufferFormat vertexDataFormat,
    SRigidVertexGPULayout*& rigidVertexData,
    SSkinnedVertexGPULayout*& skinnedVertexData,
    u32*& indices,
    usize vertexCount,
    usize indexCount
)
{
    if (vertexDataFormat == EVertexBufferFormat::Rigid_48)
        rigidVertexData = (SRigidVertexGPULayout*)CObjectAllocator::Allocate(
            vertexCount * sizeof(SRigidVertexGPULayout),
            64
        );
    else if (vertexDataFormat == EVertexBufferFormat::Skinned_80)
        skinnedVertexData = (SSkinnedVertexGPULayout*)CObjectAllocator::Allocate(
            vertexCount * sizeof(SSkinnedVertexGPULayout),
            64
        );
    indices = (u32*)CObjectAllocator::Allocate(indexCount * sizeof(u32), 64);
}

void triton::XModel3DBackendAssimp::AllocateTempBitangentBuffer(cVector3*& bitangents, usize vertexCount)
{
    bitangents = (cVector3*)CObjectAllocator::Allocate(vertexCount * sizeof(cVector3), 64);
}

void triton::XModel3DBackendAssimp::ParseVertexData(
    const aiScene* scene,
    EVertexBufferFormat vertexDataFormat,
    SRigidVertexGPULayout*& rigidVertexData,
    SSkinnedVertexGPULayout*& skinnedVertexData,
    cVector3* bitangents
)
{
    usize globalVertexIndex = 0;
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        for (usize vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
        {
            const aiVector3D position = mesh->HasPositions() ? mesh->mVertices[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D texcoord = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][vertexIndex] : aiVector3D(0.0f);
            const aiVector3D normal = mesh->HasNormals() ? mesh->mNormals[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D tangent = mesh->HasTangentsAndBitangents() ? mesh->mTangents[vertexIndex] : aiVector3D(0.0f);
            const aiVector3D bitangent = mesh->HasTangentsAndBitangents() ? mesh->mBitangents[vertexIndex] : aiVector3D(0.0f);
            const s32 materialIndex = mesh->mMaterialIndex;

            if (vertexDataFormat == EVertexBufferFormat::Rigid_48)
            {
                rigidVertexData[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
                rigidVertexData[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
                rigidVertexData[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
                rigidVertexData[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            }
            else if (vertexDataFormat == EVertexBufferFormat::Skinned_80)
            {
                skinnedVertexData[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
                skinnedVertexData[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
                skinnedVertexData[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
                skinnedVertexData[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            }

            bitangents[globalVertexIndex] = cVector3(bitangent.x, bitangent.y, bitangent.z);
            
            globalVertexIndex += 1;
        }
    }
}

void triton::XModel3DBackendAssimp::CalculateHandedness(
    EVertexBufferFormat vertexDataFormat,
    SRigidVertexGPULayout*& rigidVertexData,
    SSkinnedVertexGPULayout*& skinnedVertexData,
    cVector3* bitangents,
    usize vertexCount
)
{
    for (usize i = 0; i < vertexCount; i++)
    {
        // TODO: encapsulate GLM code to math-related backend
        if (vertexDataFormat == EVertexBufferFormat::Rigid_48)
        {
            glm::vec3 normal = glm::vec3(
                rigidVertexData[i].normal.GetX(),
                rigidVertexData[i].normal.GetY(),
                rigidVertexData[i].normal.GetZ()
            );
            glm::vec3 tangent = glm::vec3(
                rigidVertexData[i].tangent.GetX(),
                rigidVertexData[i].tangent.GetY(),
                rigidVertexData[i].tangent.GetZ()
            );
            glm::vec3 bitangent = glm::vec3(
                bitangents[i].GetX(),
                bitangents[i].GetY(),
                bitangents[i].GetZ()
            );
            f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

            rigidVertexData[i].tangent.SetW(handedness);
        }
        else if (vertexDataFormat == EVertexBufferFormat::Skinned_80)
        {
            glm::vec3 normal = glm::vec3(
                skinnedVertexData[i].normal.GetX(),
                skinnedVertexData[i].normal.GetY(),
                skinnedVertexData[i].normal.GetZ()
            );
            glm::vec3 tangent = glm::vec3(
                skinnedVertexData[i].tangent.GetX(),
                skinnedVertexData[i].tangent.GetY(),
                skinnedVertexData[i].tangent.GetZ()
            );
            glm::vec3 bitangent = glm::vec3(
                bitangents[i].GetX(),
                bitangents[i].GetY(),
                bitangents[i].GetZ()
            );
            f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

            skinnedVertexData[i].tangent.SetW(handedness);
        }
    }
}

void triton::XModel3DBackendAssimp::ParseIndexData(const aiScene* scene, u32* indexData, const std::vector<usize>& indexOffsets)
{
    usize globalIndicesIndex = 0;
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (mesh->HasFaces())
        {
            for (usize faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
            {
                const aiFace& face = mesh->mFaces[faceIndex];
                for (usize indicesIndex = 0; indicesIndex < face.mNumIndices; indicesIndex++)
                {
                    indexData[globalIndicesIndex] = indexOffsets.at(meshIndex) + face.mIndices[indicesIndex];
                    globalIndicesIndex += 1;
                }
            }
        }
    }
}

void triton::XModel3DBackendAssimp::ParseMaterialData(const aiScene* scene, std::vector<SModel3DMaterialData>& materials)
{
    for (usize materialIndex = 0; materialIndex < scene->mNumMaterials; materialIndex++)
    {
        const aiMaterial* material = scene->mMaterials[materialIndex];
        aiString diffuseTexturePath = aiString("");
        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
        aiString normalTexturePath = aiString("");
        material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath);
        aiString roughnessTexturePath = aiString("");
        material->GetTexture(aiTextureType_SHININESS, 0, &roughnessTexturePath);
        aiString metallicTexturePath = aiString("");
        material->GetTexture(aiTextureType_METALNESS, 0, &metallicTexturePath);

        SModel3DMaterialData m3dmd;
        m3dmd.diffuseTextureFilePath = diffuseTexturePath.C_Str();
        m3dmd.normalTextureFilePath = normalTexturePath.C_Str();
        m3dmd.roughnessTextureFilePath = roughnessTexturePath.C_Str();
        m3dmd.metallicTextureFilePath = metallicTexturePath.C_Str();
        m3dmd.bIsDiffuseEmbedded = scene->GetEmbeddedTexture(diffuseTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsNormalEmbedded = scene->GetEmbeddedTexture(normalTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsRoughnessEmbedded = scene->GetEmbeddedTexture(roughnessTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        m3dmd.bIsMetallicEmbedded = scene->GetEmbeddedTexture(metallicTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        materials.push_back(m3dmd);

        for (usize i = 0; i < material->mNumProperties; i++)
        {
            std::cout <<
                "name(" << material->mProperties[i]->mKey.C_Str() << ") " <<
                "semantic(" << material->mProperties[i]->mSemantic << ") " <<
                "dataLength(" << material->mProperties[i]->mDataLength << ") " <<
                "index(" << material->mProperties[i]->mIndex << ") " <<
                "type(" << material->mProperties[i]->mType << ") ";
            for (usize j = 0; j < material->mProperties[i]->mDataLength; j++)
                std::cout << material->mProperties[i]->mData[j];
            std::cout << "\n";
        }

        for (int t = 0; t <= aiTextureType_TRANSMISSION; ++t)
        {
            auto type = (aiTextureType)t;
            unsigned count = material->GetTextureCount(type);

            if (count)
            {
                printf("TYPE %d:\n", t);

                for (unsigned i = 0; i < count; ++i)
                {
                    aiString path;
                    material->GetTexture(type, i, &path);
                    printf("    %s\n", path.C_Str());
                }
            }
        }
    }
}

void triton::XModel3DBackendAssimp::CreateMaterials(
    cContext* context,
    const std::string& modelFolderPath,
    CTextureAtlas* textureAtlas,
    const std::vector<SModel3DMaterialData>& materials,
    std::vector<XMaterial::THandle>& modelMaterials,
    const aiScene* scene
)
{
    for (auto& material : materials)
    {
        auto diffOpt = CreateTexture(
            context,
            ETextureFormat::RGBA8_SRGB_Mips,
            modelFolderPath,
            textureAtlas,
            material.diffuseTextureFilePath,
            material.bIsDiffuseEmbedded,
            scene->GetEmbeddedTexture(material.diffuseTextureFilePath.c_str())
        );
        auto normOpt = CreateTexture(
            context,
            ETextureFormat::RGBA8,
            modelFolderPath,
            textureAtlas,
            material.normalTextureFilePath,
            material.bIsNormalEmbedded,
            scene->GetEmbeddedTexture(material.normalTextureFilePath.c_str())
        );
        auto rghnOpt = CreateTexture(
            context,
            ETextureFormat::R8,
            modelFolderPath,
            textureAtlas,
            material.roughnessTextureFilePath,
            material.bIsRoughnessEmbedded,
            scene->GetEmbeddedTexture(material.roughnessTextureFilePath.c_str())
        );
        auto metlOpt = CreateTexture(
            context,
            ETextureFormat::R8,
            modelFolderPath,
            textureAtlas,
            material.metallicTextureFilePath,
            material.bIsMetallicEmbedded,
            scene->GetEmbeddedTexture(material.metallicTextureFilePath.c_str())
        );

        modelMaterials.push_back(
            *context->GetPool<CMaterialPool>()->Create(
                diffOpt.has_value() ? *diffOpt : XAtlasTexture::THandle(),
                normOpt.has_value() ? *normOpt : XAtlasTexture::THandle(),
                rghnOpt.has_value() ? *rghnOpt : XAtlasTexture::THandle(),
                metlOpt.has_value() ? *metlOpt : XAtlasTexture::THandle(),
                cVector4(1.0f),
                cVector4(1.0f),
                0.0f
            )
        );
    }
}

void triton::XModel3DBackendAssimp::DeallocateTempBitangentBuffer(cVector3* bitangents)
{
    CObjectAllocator::Deallocate(bitangents);
}

void triton::XModel3DBackendAssimp::CreateBones(
    const aiScene* scene,
    SSkinnedVertexGPULayout* vertexData,
    usize vertexCount,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SSkeletonBone>& bones,
    std::vector<std::vector<SBoneWeight>>& vertexWeights
)
{
    vertexWeights.resize(vertexCount);
    usize vertexOffset = 0;
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        for (usize boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
        {
            const aiBone* bone = mesh->mBones[boneIndex];

            std::string boneName = bone->mName.C_Str();
            usize realBoneIndex = 0;
            if (boneIndices.find(boneName) == boneIndices.end())
            {
                SSkeletonBone b = {};
                b.name = boneName;
                b.modelMatrix = ConvertMatrix(bone->mOffsetMatrix);

                realBoneIndex = bones.size();
                boneIndices.insert({ boneName, realBoneIndex });
                bones.push_back(std::move(b));
            }
            else
            {
                realBoneIndex = boneIndices[boneName];
            }

            for (usize weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
            {
                const aiVertexWeight& vw = bone->mWeights[weightIndex];
                const usize vertexIndex = vertexOffset + vw.mVertexId;
                vertexWeights[vertexIndex].push_back({ 
                    realBoneIndex,
                    vw.mWeight
                });
            }
        }
        vertexOffset += mesh->mNumVertices;
    }
}

void triton::XModel3DBackendAssimp::FinalizeBoneWeights(
    cContext* context,
    SSkinnedVertexGPULayout* vertexData,
    types::usize vertexCount,
    std::vector<std::vector<SBoneWeight>>& vertexWeights
)
{
    for (usize vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
    {
        auto& weights = vertexWeights[vertexIndex];

        std::sort(
            weights.begin(),
            weights.end(),
            [](const SBoneWeight& a, const SBoneWeight& b)
            {
                return a.weight > b.weight;
            }
        );

        const sCapabilities& caps = context->GetSubsystem<CEngine>()->GetApplication()->GetCapabilities();
        if (weights.size() > caps.maxBoneCountPerVertex)
            weights.resize(caps.maxBoneCountPerVertex);

        f32 sum = 0.0f;
        for (const auto& w : weights)
            sum += w.weight;
        if (sum > 0.0f)
        {
            for (auto& w : weights)
                w.weight /= sum;
        }

        for (usize i = 0; i < weights.size(); i++)
        {
            vertexData[vertexIndex].boneIndices[i] = weights[i].boneIndex;
            vertexData[vertexIndex].boneWeights[i] = weights[i].weight;

            vertexData[vertexIndex].boneWeights[0] = 1.0f;
            vertexData[vertexIndex].boneWeights[1] = 0.0f;
            vertexData[vertexIndex].boneWeights[2] = 0.0f;
            vertexData[vertexIndex].boneWeights[3] = 0.0f;
        }
    }
}

boolean triton::XModel3DBackendAssimp::AccumulateRootTransform(
    const aiNode* node,
    aiNode*& boneRootNode,
    const aiMatrix4x4& parentRootTransform,
    aiMatrix4x4& accumulatedRootTransform,
    std::unordered_map<std::string, usize>& boneIndices
)
{
    aiMatrix4x4 current = parentRootTransform * node->mTransformation;
    auto it = boneIndices.find(node->mName.C_Str());
    if (it != boneIndices.end())
    {
        boneRootNode = (aiNode*)node;
        accumulatedRootTransform = parentRootTransform;
        return K_TRUE;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        if (AccumulateRootTransform(
            node->mChildren[i],
            boneRootNode,
            current,
            accumulatedRootTransform,
            boneIndices
        ))
            return K_TRUE;
    }

    return K_FALSE;
}

void triton::XModel3DBackendAssimp::CreateBoneHierarchy(
    const aiNode* node,
    s32 parentBone,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SSkeletonBone>& bones
)
{
    s32 currentBone = -1;

    auto it = boneIndices.find(node->mName.C_Str());
    if (it != boneIndices.end())
    {
        currentBone = it->second;

        bones[currentBone].localParentBoneIndex = parentBone;
        bones[currentBone].localMatrix = ConvertMatrix(node->mTransformation);

        if (parentBone != -1)
            bones[parentBone].localChildBoneIndices.push_back(currentBone);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        CreateBoneHierarchy(
            node->mChildren[i],
            currentBone == -1 ? parentBone : currentBone,
            boneIndices,
            bones
        );
    }
}

void triton::XModel3DBackendAssimp::CreateSkeleton(
    cContext* context,
    XSkeleton::THandle& modelSkeleton,
    const std::vector<SSkeletonBone>& bones,
    const aiMatrix4x4& accumulatedRootTransform
)
{
    modelSkeleton = *context->GetPool<CSkeletonPool>()->Create(
        bones,
        ConvertMatrix(accumulatedRootTransform)
    );
}

void triton::XModel3DBackendAssimp::CreateAnimations(
    cContext* context,
    const aiScene* scene,
    const std::unordered_map<std::string, usize>& boneIndices,
    XSkeleton::THandle modelSkeleton,
    std::vector<XAnimation::THandle>& modelAnimations
)
{
    for (usize animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
    {
        const aiAnimation* srcAnim = scene->mAnimations[animIdx];

        auto animationName = srcAnim->mName.C_Str();
        auto animationDuration = srcAnim->mDuration;
        auto animationTicksPerSecond = srcAnim->mTicksPerSecond;
        auto animationKeys = std::vector<SAnimationKey>();
        animationKeys.reserve(srcAnim->mNumChannels);

        for (usize channelIdx = 0; channelIdx < srcAnim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = srcAnim->mChannels[channelIdx];

            auto it = boneIndices.find(channel->mNodeName.C_Str());
            if (it == boneIndices.end())
                continue;

            SAnimationKey animKey = {};
            animKey.localBoneIndex = it->second;

            // Position
            animKey.positionKeys.reserve(channel->mNumPositionKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumPositionKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mPositionKeys[keyIdx];
                SBonePositionKey bpk = {};
                bpk.time = key.mTime;
                bpk.position = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.positionKeys.push_back(bpk);
            }

            // Rotation
            animKey.rotationKeys.reserve(channel->mNumRotationKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumRotationKeys; ++keyIdx)
            {
                const aiQuatKey& key = channel->mRotationKeys[keyIdx];
                SBoneRotationKey brk = {};
                brk.time = key.mTime;
                brk.rotation = cQuaternion(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.rotationKeys.push_back(brk);
            }

            // Scale
            animKey.scaleKeys.reserve(channel->mNumScalingKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumScalingKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mScalingKeys[keyIdx];
                SBoneScaleKey bsk = {};
                bsk.time = key.mTime;
                bsk.scale = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                animKey.scaleKeys.push_back(bsk);
            }

            animationKeys.push_back(std::move(animKey));
        }

        modelAnimations.push_back(
            *context->GetPool<CAnimationPool>()->Create(
                animationName,
                animationDuration,
                animationTicksPerSecond,
                animationKeys
            )
        );
    }
}

std::optional<triton::XAtlasTexture::THandle> triton::XModel3DBackendAssimp::CreateTexture(
    cContext* context,
    ETextureFormat dataFormat,
    const std::string& modelFolderPath,
    CTextureAtlas* textureAtlas,
    const std::string& textureFilePath,
    boolean bIsEmbedded,
    const aiTexture* texture
)
{
    if (bIsEmbedded == K_TRUE)
        return CreateTextureFromModelData(context, dataFormat, textureAtlas, textureFilePath, texture);
    else
        return CreateTextureFromFile(context, dataFormat, modelFolderPath, textureAtlas, textureFilePath);
}

std::optional<triton::XAtlasTexture::THandle> triton::XModel3DBackendAssimp::CreateTextureFromModelData(
    cContext* context,
    ETextureFormat dataFormat,
    CTextureAtlas* textureAtlas,
    const std::string& textureFilePath,
    const aiTexture* texture
)
{
    if (!texture)
        return std::nullopt;

    if (texture->mHeight == 0)
    {
        // Compressed PNG/JPG/DDS
        const u8* fileData = (u8*)texture->pcData;
        size_t fileByteSize = texture->mWidth;

        const char* fmtHint = &texture->achFormatHint[0];
        if (fmtHint[0] == 'p' && fmtHint[1] == 'n' && fmtHint[2] == 'g')
        {
            return context->GetPool<CAtlasTexturePool>()->Create(
                fileData,
                fileByteSize,
                0,
                0,
                0,
                ETextureFileFormat::PNG,
                dataFormat
            );
        }
        else if (fmtHint[0] == 'd' && fmtHint[1] == 'd' && fmtHint[2] == 's')
        {
            return context->GetPool<CAtlasTexturePool>()->Create(
                fileData,
                fileByteSize,
                0,
                0,
                0,
                ETextureFileFormat::DDS,
                dataFormat
            );
        }
        else
        {
            Print("Error: incorrect embedded texture format, only PNG/DDS embedded textures are supported");
            return std::nullopt;
        }
    }
    else
    {
        // Uncompressed BGRA
        // TODO: Implement embedded BGRA textures support
        const aiTexel* pixels = texture->pcData;
        unsigned width = texture->mWidth;
        unsigned height = texture->mHeight;
        Print("Error: uncompressed BGRA embedded textures are currenly unsupported, path: " + textureFilePath + "\n");
        
        return std::nullopt;
    }
}

std::optional<triton::XAtlasTexture::THandle> triton::XModel3DBackendAssimp::CreateTextureFromFile(
    cContext* context,
    ETextureFormat dataFormat,
    const std::string& modelFolderPath,
    CTextureAtlas* textureAtlas,
    const std::string& textureFilePath
)
{
    std::string newTextureFilePath;
    std::filesystem::path path(textureFilePath);
    if (textureFilePath.length() > 0 && textureFilePath.at(0) == '*')
    {
        Print("Error: embedded textures are not supported, model folder path: " + modelFolderPath);
        return std::nullopt;
    }
    else if (path.is_absolute())
    {
        newTextureFilePath =
            std::filesystem::path(modelFolderPath).generic_string() +
            "/" +
            path.filename().string();
    }
    else
    {
        path = modelFolderPath / path;
        newTextureFilePath = path.generic_string();
    }
    
    return context->GetPool<CAtlasTexturePool>()->Create(newTextureFilePath, dataFormat);
}

triton::SModel3DData triton::XModel3DBackendAssimp::PrepareResult(
    EVertexBufferFormat vertexDataFormat,
    const SRigidVertexGPULayout* rigidVertexData,
    const SSkinnedVertexGPULayout* skinnedVertexData,
    const u32* indexData,
    usize vertexCount,
    usize indexCount,
    const std::vector<XMaterial::THandle>& modelMaterials,
    XSkeleton::THandle modelSkeleton,
    const std::vector<XAnimation::THandle>& modelAnimations
)
{
    SModel3DData m3dd;
    m3dd.vertexDataFormat = vertexDataFormat;
    m3dd.rigidVertexData = rigidVertexData;
    m3dd.skinnedVertexData = skinnedVertexData;
    m3dd.indexData = indexData;
    m3dd.vertexCount = vertexCount;
    m3dd.indexCount = indexCount;
    m3dd.materials = modelMaterials;
    m3dd.skeleton = modelSkeleton;
    m3dd.animations = modelAnimations;

    return m3dd;
}

triton::cMatrix4 triton::XModel3DBackendAssimp::ConvertMatrix(const aiMatrix4x4& m)
{
    glm::mat4 result;
    result[0] = glm::vec4(m.a1, m.b1, m.c1, m.d1);
    result[1] = glm::vec4(m.a2, m.b2, m.c2, m.d2);
    result[2] = glm::vec4(m.a3, m.b3, m.c3, m.d3);
    result[3] = glm::vec4(m.a4, m.b4, m.c4, m.d4);

    return cMatrix4(result);
}