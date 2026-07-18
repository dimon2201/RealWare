// TRF.cpp

#include <fstream>
#include <vector>
#include <cstdlib>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "TRF.hpp"
#include "log.hpp"
#include "vertex.hpp"

using namespace triton::resource_file;
using namespace types;

template <EResourceFormat TResourceFormat>
CResourceFile<TResourceFormat>::CResourceFile(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        Print("Error: failed to open file '" + filePath.string() + "'");
        return;
    }

    _dataFolderPath = filePath.parent_path().generic_string();
    _dataByteSize = (types::usize)file.tellg();

    file.seekg(0, std::ios::beg);

    _data = new types::u8[_dataByteSize];

    if (!file.read((char*)_data, _dataByteSize))
    {
        delete[] _data;
        _data = nullptr;
        _dataByteSize = 0;

        Print("Error: failed to read file '" + filePath.string() + "'");
    }
}

template <EResourceFormat TResourceFormat>
CResourceFile<TResourceFormat>::~CResourceFile()
{
    if (_data)
        delete[] _data;
    _dataByteSize = 0;
}

template <EResourceFormat TResourceFormat>
void CResourceFile<TResourceFormat>::Parse()
{
    switch (TResourceFormat)
    {
        case EResourceFormat::Model3D:
        {
            ParseModel3D();
            break;
        }
    }
}

template <EResourceFormat TResourceFormat>
std::optional<SModel3DData> CResourceFile<TResourceFormat>::ParseModel3D()
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ////////// Import Assimp scene //////////
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    scene = importer.ReadFileFromMemory(
        _data,
        _dataByteSize,
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );
    if (!scene)
        Print("Error: can't load 3d model from byte data");

    ////////// Prepare temp memory //////////
    /*std::unordered_map<std::string, types::usize> boneIndices = {};
    std::vector<SBone> bones = {};
    std::vector<std::vector<SBoneWeight>> vertexWeights;
    std::vector<SAnimation> animations = {};
    HSkeleton modelSkeleton = {};
    usize boneOffset = 0;
    std::vector<HAnimation> modelAnimations = {};
    aiNode* boneRootNode = nullptr;
    aiMatrix4x4 parentRootTransform = aiMatrix4x4();
    aiMatrix4x4 accumulatedRootTransform = aiMatrix4x4();*/

    ////////// Count vertices and indices //////////
    usize vertexCount = 0;
    usize indexCount = 0;
    std::vector<usize> indexOffsets = {};
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        indexOffsets.push_back(vertexCount);
        vertexCount += scene->mMeshes[meshIndex]->mNumVertices;
    }
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
        if (scene->mMeshes[meshIndex]->HasFaces())
            for (usize faceIndex = 0; faceIndex < scene->mMeshes[meshIndex]->mNumFaces; faceIndex++)
                indexCount += scene->mMeshes[meshIndex]->mFaces[faceIndex].mNumIndices;

    ////////// Allocate vertex and index buffers //////////
    SVertex* vertexData = (SVertex*)malloc(vertexCount * sizeof(SVertex));
    u32* indices = (u32*)malloc(indexCount * sizeof(u32));

    ////////// Allocate temporary bitangent buffer //////////
    cVector3* bitangents = (cVector3*)malloc(vertexCount * sizeof(cVector3));
    
    ////////// Parse vertex data //////////
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
    
    ////////// Calculate handness //////////
    for (usize i = 0; i < vertexCount; i++)
    {
        glm::vec3 normal = glm::vec3(vertexData[i].normal.GetX(), vertexData[i].normal.GetY(), vertexData[i].normal.GetZ());
        glm::vec3 tangent = glm::vec3(vertexData[i].tangent.GetX(), vertexData[i].tangent.GetY(), vertexData[i].tangent.GetZ());
        glm::vec3 bitangent = glm::vec3(bitangents[i].GetX(), bitangents[i].GetY(), bitangents[i].GetZ());
        f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
        vertexData[i].tangent.SetW(handedness);
    }
    
    ////////// Deallocate temp bitangent buffer //////////
    free(bitangents);

    ////////// Parse material data //////////
    std::vector<SModel3DMaterialData> materials = {};
    for (usize materialIndex = 0; materialIndex < scene->mNumMaterials; materialIndex++)
    {
        const aiMaterial* material = scene->mMaterials[materialIndex];

        aiString diffuseTexturePath = aiString("");
        aiString normalTexturePath = aiString("");
        aiString roughnessTexturePath = aiString("");
        aiString metallicTexturePath = aiString("");
        material->GetTexture(aiTextureType_DIFFUSE, 0, &diffuseTexturePath);
        material->GetTexture(aiTextureType_NORMALS, 0, &normalTexturePath);
        material->GetTexture(aiTextureType_SHININESS, 0, &roughnessTexturePath);
        material->GetTexture(aiTextureType_METALNESS, 0, &metallicTexturePath);

        boolean bIsDiffuseEmbedded = scene->GetEmbeddedTexture(diffuseTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        boolean bIsNormalEmbedded = scene->GetEmbeddedTexture(normalTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        boolean bIsRoughnessEmbedded = scene->GetEmbeddedTexture(roughnessTexturePath.C_Str()) ? K_TRUE : K_FALSE;
        boolean bIsMetallicEmbedded = scene->GetEmbeddedTexture(metallicTexturePath.C_Str()) ? K_TRUE : K_FALSE;

        SModel3DMaterialData m3dmd;
        m3dmd.diffuseTexturePath = diffuseTexturePath.C_Str();
        m3dmd.normalTexturePath = normalTexturePath.C_Str();
        m3dmd.roughnessTexturePath = roughnessTexturePath.C_Str();
        m3dmd.metallicTexturePath = metallicTexturePath.C_Str();
        m3dmd.bIsDiffuseEmbedded = bIsDiffuseEmbedded;
        m3dmd.bIsNormalEmbedded = bIsNormalEmbedded;
        m3dmd.bIsRoughnessEmbedded = bIsRoughnessEmbedded;
        m3dmd.bIsMetallicEmbedded = bIsMetallicEmbedded;

        materials.push_back(m3dmd);
    }

    ////////// Create materials //////////
    for (auto& material : materials)
    {
        auto diffOpt = ParseModel3DTexture(
            material.bIsDiffuseEmbedded,
            scene->GetEmbeddedTexture(material.diffuseTexturePath.c_str()),
            _dataFolderPath,
            material.diffuseTexturePath
        );
        auto normOpt = ParseModel3DTexture(
            material.bIsNormalEmbedded,
            scene->GetEmbeddedTexture(material.normalTexturePath.c_str()),
            _dataFolderPath,
            material.normalTexturePath
        );
        auto rghnOpt = ParseModel3DTexture(
            material.bIsRoughnessEmbedded,
            scene->GetEmbeddedTexture(material.roughnessTexturePath.c_str()),
            _dataFolderPath,
            material.roughnessTexturePath
        );
        auto metlOpt = ParseModel3DTexture(
            material.bIsMetallicEmbedded,
            scene->GetEmbeddedTexture(material.metallicTexturePath.c_str()),
            _dataFolderPath,
            material.metallicTexturePath
        );

        if (!diffOpt.has_value())
        {
            Print("Error: can't create material diffuse texture");
            
            return std::nullopt;
        }
        else
        {
            Print("Info: material diffuse texture OK");
        }

        if (!normOpt.has_value())
        {
            Print("Error: can't create material normal texture");

            return std::nullopt;
        }
        else
        {
            Print("Info: material normal texture OK");
        }

        if (!rghnOpt.has_value())
        {
            Print("Error: can't create material roughness texture");

            return std::nullopt;
        }
        else
        {
            Print("Info: material roughness texture OK");
        }

        if (!metlOpt.has_value())
        {
            Print("Error: can't create material metallic texture");

            return std::nullopt;
        }
        else
        {
            Print("Info: material metallic texture OK");
        }

        material.diffuseTexture = *diffOpt;
        material.normalTexture = *normOpt;
        material.roughnessTexture = *rghnOpt;
        material.metallicTexture = *metlOpt;
    }

    ////////// Set absolute material indices //////////
    /*for (usize i = 0; i < vertexCount; i++)
        vertexData[i].materialIndex =
        _context->GetSubsystem<XMaterialSubsystem>()->GetHandleBufferIndex(
            modelMaterials.at(vertexData[i].materialIndex)
        );

    ////////// Parse index data //////////
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

    ////////// Create bones //////////
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
    
    ////////// Finalize bone weights //////////
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

            vertexData[vertexIndex].boneWeights[0] = 1.0f;
            vertexData[vertexIndex].boneWeights[1] = 0.0f;
            vertexData[vertexIndex].boneWeights[2] = 0.0f;
            vertexData[vertexIndex].boneWeights[3] = 0.0f;
        }
    }
    
    ////////// Accumulate bone transform //////////
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
    
    ////////// Create bone hierarchy //////////
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
    
    ////////// Create skeleton //////////
    modelSkeleton = skeletonSubsystem->CreateSkeleton(
        bones,
        ConvertMatrix(accumulatedRootTransform)
    );
    
    ////////// Create animations //////////
    for (usize animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
    {
        const aiAnimation* srcAnim = scene->mAnimations[animIdx];

        SAnimation animation = {};
        animation.name = srcAnim->mName.C_Str();
        animation.duration = srcAnim->mDuration;
        animation.ticksPerSecond = srcAnim->mTicksPerSecond;
        animation.animKeys.reserve(srcAnim->mNumChannels);

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

            animation.animKeys.push_back(std::move(animKey));
        }

        modelAnimations.push_back(
            animationSubsystem->CreateAnimation(
                animation.name,
                animation.duration,
                animation.ticksPerSecond,
                modelSkeleton,
                animation.animKeys
            ));
    }

    ////////// Return //////////
    SModel3DData m3dd;
    m3dd.vertexData = vertexData;
    m3dd.indexData = indexData;
    m3dd.vertexCount = vertexCount;
    m3dd.indexCount = indexCount;
    m3dd.materials = modelMaterials;
    m3dd.skeleton = modelSkeleton;
    m3dd.animations = modelAnimations;*/

    return std::nullopt;
}

template <EResourceFormat TResourceFormat>
std::optional<STextureData> CResourceFile<TResourceFormat>::ParseModel3DTexture(
    boolean bIsEmbeddedTexture,
    const aiTexture* assimpTexture,
    const std::filesystem::path& modelFolderPath,
    const std::filesystem::path& textureLocalFilePath
)
{
    STextureData td;

    if (bIsEmbeddedTexture == K_TRUE)
    {
        ////////// From model byte data //////////
        if (!assimpTexture)
            return std::nullopt;

        if (assimpTexture->mHeight == 0)
        {
            const u8* textureData = (u8*)assimpTexture->pcData;
            size_t textureByteSize = assimpTexture->mWidth;

            if (assimpTexture->achFormatHint[0] == 'p' ||
                assimpTexture->achFormatHint[1] == 'n' ||
                assimpTexture->achFormatHint[2] == 'g')
            {
                // PNG
                Print("Info: texture format identified as PNG, reading...");

                int width;
                int height;
                int channels;
                stbi_uc* pixels = stbi_load_from_memory(
                    textureData,
                    (int)textureByteSize,
                    &width,
                    &height,
                    &channels,
                    0
                );

                if (ParsePNG(
                    (usize)width,
                    (usize)height,
                    (usize)channels,
                    (u8*)pixels,
                    td.format,
                    td.dataFormat,
                    td.width,
                    td.height,
                    td.channelCount,
                    td.pixelByteData
                ) == K_FALSE)
                {
                    Print("Error: can't create PNG texture");

                    return std::nullopt;
                }
            }
            else if (assimpTexture->achFormatHint[0] == 'd' ||
                assimpTexture->achFormatHint[1] == 'd' ||
                assimpTexture->achFormatHint[2] == 's')
            {
                // DDS
                Print("Info: texture format identified as DDS, reading...");

                tinyddsloader::DDSFile dds;
                auto result = dds.Load(textureData, textureByteSize);
                if (result != tinyddsloader::Result::Success)
                {
                    Print("Error: can't read DDS texture");

                    return std::nullopt;
                }
                uint32_t width = dds.GetWidth();
                uint32_t height = dds.GetHeight();
                uint32_t channels = 0;
                tinyddsloader::DDSFile::DXGIFormat format = dds.GetFormat();
                void* pixels = dds.GetImageData()->m_mem;
                
                if (ParseDDS(
                    (usize)width,
                    (usize)height,
                    format,
                    (u8*)pixels,
                    td.format,
                    td.dataFormat,
                    td.width,
                    td.height,
                    td.channelCount,
                    td.pixelByteData
                ) == K_FALSE)
                {
                    Print("Error: can't create DDS texture");

                    return std::nullopt;
                }
            }
            else
            {
                Print("Error: unsupported texture format '" + std::string(&assimpTexture->achFormatHint[0]) + "'");

                return std::nullopt;
            }
        }
        else
        {
            // Raw BGRA
            Print("Info: texture format identified as raw BGRA, reading...");

            const aiTexel* pixels = assimpTexture->pcData;
            unsigned width = assimpTexture->mWidth;
            unsigned height = assimpTexture->mHeight;
            Print("Error: raw BGRA textures are currenly unsupported");

            return std::nullopt;
        }
    }
    else
    {
        ////////// From file //////////
        const std::string textureFilePath = (modelFolderPath / textureLocalFilePath).generic_string();

        if (!std::filesystem::exists(textureFilePath))
        {
            Print("Error: texture file '" + textureFilePath + "' doesn't exist");

            return std::nullopt;
        }

        if (!(textureLocalFilePath.extension() == ".png" ||
            textureLocalFilePath.extension() == ".dds"))
        {
            Print("Error: unsupported texture format '" + textureLocalFilePath.extension().generic_string() + "'");

            return std::nullopt;
        }

        if (textureLocalFilePath.extension() == ".png")
        {
            // PNG
            Print("Info: texture format identified as PNG, reading...");

            int width;
            int height;
            int channels;
            stbi_uc* pixels = stbi_load(
                textureFilePath.c_str(),
                &width,
                &height,
                &channels,
                0
            );

            if (ParsePNG(
                (usize)width,
                (usize)height,
                (usize)channels,
                (u8*)pixels,
                td.format,
                td.dataFormat,
                td.width,
                td.height,
                td.channelCount,
                td.pixelByteData
            ) == K_FALSE)
            {
                Print("Error: can't create PNG texture");

                return std::nullopt;
            }
        }
        else if (textureLocalFilePath.extension() == ".dds")
        {
            Print("Info: texture format identified as DDS, reading...");

            tinyddsloader::DDSFile dds;
            auto result = dds.Load(textureFilePath.c_str());
            uint32_t width = dds.GetWidth();
            uint32_t height = dds.GetHeight();
            uint32_t channels = 0;
            tinyddsloader::DDSFile::DXGIFormat format = dds.GetFormat();
            void* pixels = dds.GetImageData()->m_mem;

            if (ParseDDS(
                (usize)width,
                (usize)height,
                format,
                (u8*)pixels,
                td.format,
                td.dataFormat,
                td.width,
                td.height,
                td.channelCount,
                td.pixelByteData
            ) == K_FALSE)
            {
                Print("Error: can't create DDS texture");

                return std::nullopt;
            }
        }
        else
        {
            std::string textureFormatHint = std::string(&assimpTexture->achFormatHint[0]);
            Print("Error: unsupported texture format '" + textureFormatHint + "'");

            return std::nullopt;
        }
    }

    return td;
}

template <EResourceFormat TResourceFormat>
boolean CResourceFile<TResourceFormat>::ParsePNG(
    usize width,
    usize height,
    usize channels,
    u8* pixels,
    ETextureFormat& outFormat,
    ETextureDataFormat& outDataFormat,
    usize& outWidth,
    usize& outHeight,
    usize& outChannels,
    u8*& outPixels
)
{
    if (!pixels)
    {
        Print("Error: can't read PNG texture");

        return K_FALSE;
    }

    usize texByteSize = (usize)(width * height * channels);
    u8* texPixels = (u8*)malloc(texByteSize);
    memset(&texPixels[0], texByteSize, 0);
    memcpy(&texPixels[0], &pixels[0], texByteSize);

    stbi_image_free(pixels);

    if (channels == 1)
    {
        outDataFormat = ETextureDataFormat::R8;
    }
    else if (channels == 3)
    {
        outDataFormat = ETextureDataFormat::RGBA8;
    }
    else if (channels == 4)
    {
        outDataFormat = ETextureDataFormat::RGBA8_SRGB;
    }
    else
    {
        Print("Error: unsupported texture data format");

        return K_FALSE;
    }

    outFormat = ETextureFormat::PNG;
    outWidth = (usize)width;
    outHeight = (usize)height;
    outChannels = (usize)channels;
    outPixels = texPixels;

    return K_TRUE;
}

template <EResourceFormat TResourceFormat>
boolean CResourceFile<TResourceFormat>::ParseDDS(
    usize width,
    usize height,
    tinyddsloader::DDSFile::DXGIFormat format,
    u8* pixels,
    ETextureFormat& outFormat,
    ETextureDataFormat& outDataFormat,
    usize& outWidth,
    usize& outHeight,
    usize& outChannels,
    u8*& outPixels
)
{
    if (!pixels)
    {
        Print("Error: can't read DDS texture");

        return K_FALSE;
    }

    const usize channels = outChannels;
    usize texByteSize = (usize)(width * height * channels);
    u8* texPixels = (u8*)malloc(texByteSize);
    memset(&texPixels[0], texByteSize, 0);
    memcpy(&texPixels[0], &pixels[0], texByteSize);

    if (format == tinyddsloader::DDSFile::DXGIFormat::R8_UNorm)
    {
        outChannels = 1;
        outDataFormat = ETextureDataFormat::R8;
    }
    else if (format == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm)
    {
        outChannels = 4;
        outDataFormat = ETextureDataFormat::RGBA8;
    }
    else if (format == tinyddsloader::DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB)
    {
        outChannels = 4;
        outDataFormat = ETextureDataFormat::RGBA8_SRGB;
    }
    else
    {
        Print("Error: unsupported texture data format");

        return K_FALSE;
    }

    outFormat = ETextureFormat::DDS;
    outWidth = (usize)width;
    outHeight = (usize)height;
    outChannels = (usize)channels;
    outPixels = texPixels;

    return K_TRUE;
}

template class CResourceFile<EResourceFormat::Model3D>;