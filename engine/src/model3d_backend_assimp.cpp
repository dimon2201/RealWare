// model3d_backend_assimp.cpp

#include <filesystem>
#include "model3d_backend_assimp.hpp"
#include "model3d_backend_resource.hpp"
#include "context.hpp"
#include "vertex.hpp"
#include "math.hpp"
#include "material_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "animation_subsystem.hpp"
#include "bone.hpp"
#include "animation.hpp"

using namespace types;

std::optional<triton::SModel3DBackendResource> triton::XModel3DBackendAssimp::CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ImportScene(importer, scene, modelFolderPath + "/" + modelLocalPath);

    if (!scene)
        return std::nullopt;
    
    usize vertexCount = 0;
    usize indexCount = 0;
    std::vector<usize> indexOffsets = {};
    SVertex* vertexData = nullptr;
    u32* indexData = nullptr;
    cVector3* bitangents = nullptr;
    std::vector<SModel3DMaterialData> materials = {};
    std::vector<HMaterial> modelMaterials = {};
    std::unordered_map<std::string, types::usize> boneIndices = {};
    std::vector<SBone> bones = {};
    std::vector<std::vector<SBoneWeight>> vertexWeights;
    std::vector<SAnimation> animations = {};
    HSkeleton modelSkeleton = {};
    usize boneOffset = 0;

    CountVerticesIndices(scene, vertexCount, indexCount, indexOffsets);

    AllocateVertexIndexBuffers(vertexData, indexData, vertexCount, indexCount);
    AllocateTempBitangentBuffer(bitangents, vertexCount);
    ParseVertexData(scene, vertexData, bitangents);
    CalculateHandedness(vertexData, bitangents, vertexCount);
    DeallocateTempBitangentBuffer(bitangents);

    ParseMaterialData(scene, materials);
    CreateMaterials(
        modelFolderPath,
        _context->GetSubsystem<XTextureSubsystem>(),
        _context->GetSubsystem<XMaterialSubsystem>(), 
        materials,
        modelMaterials,
        scene
    );
    SetAbsoluteMaterialIndices(vertexData, vertexCount, modelMaterials);

    ParseIndexData(scene, indexData, indexOffsets);

    CreateBones(scene, vertexData, vertexCount, boneIndices, bones, vertexWeights);
    FinalizeBoneWeights(vertexData, vertexCount, vertexWeights);
    CreateBoneHierarchy(scene->mRootNode, -1, boneIndices, bones);
    CreateSkeleton(modelSkeleton, bones, _context->GetSubsystem<XSkeletonSubsystem>());
    CreateAnimations(scene, boneIndices, _context->GetSubsystem<XAnimationSubsystem>(), modelSkeleton);

    return PrepareResult(vertexData, indexData, vertexCount, indexCount, modelMaterials);
}

void triton::XModel3DBackendAssimp::DestroyModel(SModel3DBackendResource& mesh)
{
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
    if (mesh.indexData)
        _context->GetMemoryAllocator()->Deallocate((void*)mesh.indexData);
    if (mesh.vertexData)
        _context->GetMemoryAllocator()->Deallocate((void*)mesh.vertexData);
}

void triton::XModel3DBackendAssimp::ImportScene(Assimp::Importer& importer, const aiScene*& scene, const std::string& filePath)
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
        Print("Error: can't load mesh from file '" + filePath + "'");
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

void triton::XModel3DBackendAssimp::AllocateVertexIndexBuffers(SVertex*& vertexData, u32*& indices, usize vertexCount, usize indexCount)
{
    vertexData = (SVertex*)_context->GetMemoryAllocator()->Allocate(vertexCount * sizeof(SVertex), 64);
    indices = (u32*)_context->GetMemoryAllocator()->Allocate(indexCount * sizeof(u32), 64);
}

void triton::XModel3DBackendAssimp::AllocateTempBitangentBuffer(cVector3*& bitangents, usize vertexCount)
{
    bitangents = (cVector3*)_context->GetMemoryAllocator()->Allocate(vertexCount * sizeof(cVector3), 64);
}

void triton::XModel3DBackendAssimp::ParseVertexData(const aiScene* scene, SVertex* vertexData, cVector3* bitangents)
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

            vertexData[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
            vertexData[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
            vertexData[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
            vertexData[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            vertexData[globalVertexIndex].materialIndex = materialIndex;
            bitangents[globalVertexIndex] = cVector3(bitangent.x, bitangent.y, bitangent.z);
            
            globalVertexIndex += 1;
        }
    }
}

void triton::XModel3DBackendAssimp::CalculateHandedness(SVertex* vertexData, cVector3* bitangents, usize vertexCount)
{
    for (usize i = 0; i < vertexCount; i++)
    {
        // TODO: encapsulate GLM code to math-related backend
        glm::vec3 normal = glm::vec3(vertexData[i].normal.GetX(), vertexData[i].normal.GetY(), vertexData[i].normal.GetZ());
        glm::vec3 tangent = glm::vec3(vertexData[i].tangent.GetX(), vertexData[i].tangent.GetY(), vertexData[i].tangent.GetZ());
        glm::vec3 bitangent = glm::vec3(bitangents[i].GetX(), bitangents[i].GetY(), bitangents[i].GetZ());
        f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
        vertexData[i].tangent.SetW(handedness);
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

void triton::XModel3DBackendAssimp::CreateMaterials(const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, XMaterialSubsystem* materialSubsystem, const std::vector<SModel3DMaterialData>& materials, std::vector<HMaterial>& modelMaterials, const aiScene* scene)
{
    for (auto& material : materials)
    {
        HTexture diffuseTexture = *CreateTexture(
            cTexture::eFormat::RGBA8_SRGB_MIPS,
            modelFolderPath,
            textureSubsystem,
            material.diffuseTextureFilePath,
            material.bIsDiffuseEmbedded,
            scene->GetEmbeddedTexture(material.diffuseTextureFilePath.c_str())
        );
        HTexture normalTexture = *CreateTexture(
            cTexture::eFormat::RGBA8,
            modelFolderPath,
            textureSubsystem,
            material.normalTextureFilePath,
            material.bIsNormalEmbedded,
            scene->GetEmbeddedTexture(material.normalTextureFilePath.c_str())
        );
        HTexture roughnessTexture = *CreateTexture(
            cTexture::eFormat::R8,
            modelFolderPath,
            textureSubsystem,
            material.roughnessTextureFilePath,
            material.bIsRoughnessEmbedded,
            scene->GetEmbeddedTexture(material.roughnessTextureFilePath.c_str())
        );
        HTexture metallicTexture = *CreateTexture(
            cTexture::eFormat::R8,
            modelFolderPath,
            textureSubsystem,
            material.metallicTextureFilePath,
            material.bIsMetallicEmbedded,
            scene->GetEmbeddedTexture(material.metallicTextureFilePath.c_str())
        );

        modelMaterials.push_back(
            materialSubsystem->CreateMaterial(cVector4(1.0f),
                diffuseTexture,
                normalTexture,
                roughnessTexture,
                metallicTexture
            )
        );
    }
}

void triton::XModel3DBackendAssimp::SetAbsoluteMaterialIndices(SVertex*& vertexData, usize vertexCount, const std::vector<HMaterial>& modelMaterials)
{
    for (usize i = 0; i < vertexCount; i++)
        vertexData[i].materialIndex = modelMaterials.at(vertexData[i].materialIndex)._indexInArray;
}

void triton::XModel3DBackendAssimp::DeallocateTempBitangentBuffer(cVector3* bitangents)
{
    _context->GetMemoryAllocator()->Deallocate(bitangents);
}

void triton::XModel3DBackendAssimp::CreateBones(
    const aiScene* scene,
    SVertex* vertexData,
    usize vertexCount,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SBone>& bones,
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
                SBone b = {};
                b.name = boneName;
                b.parentBoneSpaceToThisBoneSpace = ConvertMatrix(bone->mOffsetMatrix);

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

void triton::XModel3DBackendAssimp::FinalizeBoneWeights(SVertex* vertexData, types::usize vertexCount, std::vector<std::vector<SBoneWeight>>& vertexWeights)
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

        if (weights.size() > SVertex::kMaxBonesPerVertex)
            weights.resize(SVertex::kMaxBonesPerVertex);

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
        }
    }
}

void triton::XModel3DBackendAssimp::CreateBoneHierarchy(
    const aiNode* node,
    s32 parentBone,
    std::unordered_map<std::string, usize>& boneIndices,
    std::vector<SBone>& bones
)
{
    s32 currentBone = -1;

    auto it = boneIndices.find(node->mName.C_Str());
    if (it != boneIndices.end())
    {
        currentBone = it->second;

        bones[currentBone].parentLocalBoneIndex = parentBone;
        bones[currentBone].modelSpaceToThisBoneSpace = ConvertMatrix(node->mTransformation);

        if (parentBone != -1)
            bones[parentBone].childrenLocalBoneIndices.push_back(currentBone);
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
    HSkeleton& modelSkeleton,
    const std::vector<SBone>& bones,
    XSkeletonSubsystem* skeletonSubsystem
)
{
    modelSkeleton = skeletonSubsystem->CreateSkeleton(bones);
}

void triton::XModel3DBackendAssimp::CreateAnimations(
    const aiScene* scene,
    const std::unordered_map<std::string, usize>& boneIndices,
    XAnimationSubsystem* animationSubsystem,
    HSkeleton modelSkeleton
)
{
    for (usize animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
    {
        const aiAnimation* srcAnim = scene->mAnimations[animIdx];

        SAnimation animation = {};
        animation.name = srcAnim->mName.C_Str();
        animation.duration = srcAnim->mDuration;
        animation.ticksPerSecond = srcAnim->mTicksPerSecond;
        animation.bones.reserve(srcAnim->mNumChannels);

        for (usize channelIdx = 0; channelIdx < srcAnim->mNumChannels; ++channelIdx)
        {
            const aiNodeAnim* channel = srcAnim->mChannels[channelIdx];

            auto it = boneIndices.find(channel->mNodeName.C_Str());
            if (it == boneIndices.end())
                continue;

            SBoneAnimation boneAnim;
            boneAnim.localBoneIndex = it->second;

            // Position
            boneAnim.positionKeys.reserve(channel->mNumPositionKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumPositionKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mPositionKeys[keyIdx];
                SBonePositionKey bpk = {};
                bpk.time = key.mTime;
                bpk.position = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.positionKeys.push_back(bpk);
            }

            // Rotation
            boneAnim.rotationKeys.reserve(channel->mNumRotationKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumRotationKeys; ++keyIdx)
            {
                const aiQuatKey& key = channel->mRotationKeys[keyIdx];
                SBoneRotationKey brk = {};
                brk.time = key.mTime;
                brk.rotation = cQuaternion(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.rotationKeys.push_back(brk);
            }

            // Scale
            boneAnim.scaleKeys.reserve(channel->mNumScalingKeys);
            for (usize keyIdx = 0; keyIdx < channel->mNumScalingKeys; ++keyIdx)
            {
                const aiVectorKey& key = channel->mScalingKeys[keyIdx];
                SBoneScaleKey bsk = {};
                bsk.time = key.mTime;
                bsk.scale = cVector3(key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.scaleKeys.push_back(bsk);
            }

            animation.bones.push_back(std::move(boneAnim));
        }

        animationSubsystem->CreateAnimation(
            animation.name,
            animation.duration,
            animation.ticksPerSecond,
            modelSkeleton,
            animation.bones
        );
    }
}

std::optional<triton::HTexture> triton::XModel3DBackendAssimp::CreateTexture(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, boolean bIsEmbedded, const aiTexture* texture)
{
    if (bIsEmbedded == K_TRUE)
        return CreateTextureFromModelData(dataFormat, textureSubsystem, textureFilePath, texture);
    else
        return CreateTextureFromFile(dataFormat, modelFolderPath, textureSubsystem, textureFilePath);
}

std::optional<triton::HTexture> triton::XModel3DBackendAssimp::CreateTextureFromModelData(cTexture::eFormat dataFormat, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath, const aiTexture* texture)
{
    if (!texture)
        return std::nullopt;

    if (texture->mHeight == 0)
    {
        // Compressed PNG/JPG/DDS
        const u8* fileData = (u8*)texture->pcData;
        size_t fileByteSize = texture->mWidth;

        if (texture->achFormatHint[0] != 'p' ||
            texture->achFormatHint[1] != 'n' ||
            texture->achFormatHint[2] != 'g')
        {
            Print("Error: incorrect embedded texture format, only PNG embedded textures are supported");
            return std::nullopt;
        }

        return textureSubsystem->CreateTexture(fileData, fileByteSize, ETextureFormat::PNG, dataFormat);
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

std::optional<triton::HTexture> triton::XModel3DBackendAssimp::CreateTextureFromFile(cTexture::eFormat dataFormat, const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath)
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
    
    return textureSubsystem->CreateTexture(newTextureFilePath, dataFormat);
}

triton::SModel3DBackendResource triton::XModel3DBackendAssimp::PrepareResult(const SVertex* vertexData, const u32* indexData, usize vertexCount, usize indexCount, const std::vector<HMaterial>& modelMaterials)
{
    SModel3DBackendResource mbr;
    mbr.vertexData = vertexData;
    mbr.indexData = indexData;
    mbr.vertexCount = vertexCount;
    mbr.indexCount = indexCount;
    mbr.materials = modelMaterials;

    return mbr;
}

triton::cMatrix4 triton::XModel3DBackendAssimp::ConvertMatrix(const aiMatrix4x4& m)
{
    return cMatrix4(glm::mat4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4
    ));
}