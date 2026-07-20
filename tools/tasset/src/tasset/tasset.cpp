// TRF.cpp

#include <fstream>
#include <vector>
#include <cstdlib>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/importerdesc.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>
#include "tasset.hpp"

using namespace triton::asset;
using namespace types;

std::optional<SModel3DData> ParseModel3D(
    u8* data,
    usize dataByteSize,
    const std::string& dataFolderPath,
    const std::string& dataLocalFilePath
);

std::optional<STextureData> ParseModel3DTexture(
    types::boolean bIsEmbeddedTexture,
    const aiTexture* assimpTexture,
    const std::filesystem::path& modelFolderPath,
    const std::filesystem::path& textureLocalFilePath
);

boolean DecidePNGFormat(
    usize channels,
    ETextureDataFormat& outDataFormat
);

boolean DecideDDSFormat(
    tinyddsloader::DDSFile::DXGIFormat format,
    usize& outChannels,
    ETextureDataFormat& outDataFormat
);

types::boolean AccumulateBoneTransform(
    const aiNode* root,
    aiNode*& outBoneRoot,
    const aiMatrix4x4& parentTransform,
    const std::unordered_map<std::string, types::usize>& boneIndices,
    aiMatrix4x4& outAccumulatedTransform
);

void ConstructBoneHierarchy(
    const aiNode* node,
    types::s32 parentBoneIndex,
    const std::unordered_map<std::string, types::usize>& boneIndexData,
    std::vector<SModel3DBoneData>& boneData
);

glm::mat4 ConvertMatrix(const aiMatrix4x4& matrix);

void triton::asset::Print(const std::string& message)
{
    std::cout << message << std::endl;
}

std::optional<SModel3DData> ParseModel3D(
    u8* data,
    usize dataByteSize,
    const std::string& dataFolderPath,
    const std::string& dataLocalFilePath
)
{
    Print(
        "Info: parsing 3d model resource at '" +
        dataFolderPath + "/" +
        dataLocalFilePath +
        "'..."
    );

    Assimp::Importer importer;
    const aiScene* scene = nullptr;

    ////////// Import Assimp scene //////////
    Print("Info: creating scene using Assimp importer...");

    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_NORMALS);
    scene = importer.ReadFileFromMemory(
        data,
        dataByteSize,
        // aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_ImproveCacheLocality
    );
    if (!scene)
    {
        Print("Error: can't load 3d model from byte data");

        return std::nullopt;
    }

    const aiImporterDesc* desc = importer.GetImporterInfo(0);
    Print("Info: Assimp importer info = " + std::string(desc->mName));

    ////////// Count vertices and indices //////////
    Print("Info: couting vertices and indices...");

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
    Print("Info: allocating vertex and index data buffers...");

    SVertex* vertexData = (SVertex*)malloc(vertexCount * sizeof(SVertex));
    u32* indexData = (u32*)malloc(indexCount * sizeof(u32));

    ////////// Allocate temporary bitangent buffer //////////
    Print("Info: allocating temporary bitangent data buffer...");

    glm::vec3* bitangents = (glm::vec3*)malloc(vertexCount * sizeof(glm::vec3));

    ////////// Parse vertex data //////////
    Print("Info: parsing vertex data...");

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

            vertexData[globalVertexIndex].position = glm::vec3(position.x, position.y, position.z);
            vertexData[globalVertexIndex].texcoord = glm::vec2(texcoord.x, texcoord.y);
            vertexData[globalVertexIndex].normal = glm::vec3(normal.x, normal.y, normal.z);
            vertexData[globalVertexIndex].tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 0.0f);
            vertexData[globalVertexIndex].materialIndex = materialIndex;

            bitangents[globalVertexIndex] = glm::vec3(bitangent.x, bitangent.y, bitangent.z);

            globalVertexIndex += 1;
        }
    }

    ////////// Calculate handness //////////
    Print("Info: calculating vertex data handedness...");

    for (usize i = 0; i < vertexCount; i++)
    {
        glm::vec3 normal = glm::vec3(vertexData[i].normal.x, vertexData[i].normal.y, vertexData[i].normal.z);
        glm::vec3 tangent = glm::vec3(vertexData[i].tangent.x, vertexData[i].tangent.y, vertexData[i].tangent.z);
        glm::vec3 bitangent = glm::vec3(bitangents[i].x, bitangents[i].y, bitangents[i].z);
        f32 handedness = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
        vertexData[i].tangent.w = handedness;
    }

    ////////// Deallocate temp bitangent buffer //////////
    Print("Info: deallocating temporary bitangent data buffer...");

    free(bitangents);

    ////////// Parse material data //////////
    std::vector<SModel3DMaterialData> materialData = {};
    if (scene->mNumMaterials > 0)
    {
        Print("Info: resource contains material data");

        Print("Info: parsing material data...");

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

            materialData.push_back(m3dmd);
        }

        ////////// Create materials //////////

        Print("Info: creating materials...");

        for (auto& material : materialData)
        {
            auto diffOpt = ParseModel3DTexture(
                material.bIsDiffuseEmbedded,
                scene->GetEmbeddedTexture(material.diffuseTexturePath.c_str()),
                dataFolderPath,
                material.diffuseTexturePath
            );
            auto normOpt = ParseModel3DTexture(
                material.bIsNormalEmbedded,
                scene->GetEmbeddedTexture(material.normalTexturePath.c_str()),
                dataFolderPath,
                material.normalTexturePath
            );
            auto rghnOpt = ParseModel3DTexture(
                material.bIsRoughnessEmbedded,
                scene->GetEmbeddedTexture(material.roughnessTexturePath.c_str()),
                dataFolderPath,
                material.roughnessTexturePath
            );
            auto metlOpt = ParseModel3DTexture(
                material.bIsMetallicEmbedded,
                scene->GetEmbeddedTexture(material.metallicTexturePath.c_str()),
                dataFolderPath,
                material.metallicTexturePath
            );

            Print("Info: checking textures...");

            if (!diffOpt.has_value())
            {
                Print("Error: diffuse texture is NULL");

                return std::nullopt;
            }
            else
            {
                Print("Info: diffuse texture is OK");
            }

            if (!normOpt.has_value())
            {
                Print("Error: normal texture is NULL");

                return std::nullopt;
            }
            else
            {
                Print("Info: normal texture is OK");
            }

            if (!rghnOpt.has_value())
            {
                Print("Error: roughness texture is NULL");

                return std::nullopt;
            }
            else
            {
                Print("Info: roughness texture is OK");
            }

            if (!metlOpt.has_value())
            {
                Print("Error: metallic texture is NULL");

                return std::nullopt;
            }
            else
            {
                Print("Info: metallic texture is OK");
            }

            material.diffuseTexture = *diffOpt;
            material.normalTexture = *normOpt;
            material.roughnessTexture = *rghnOpt;
            material.metallicTexture = *metlOpt;
        }
    }
    else
    {
        Print("Info: resource does not contain any material data");
    }

    ////////// Parse index data //////////
    Print("Info: parsing index data...");

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
    boolean bHasBones = K_FALSE;
    for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        const aiMesh* mesh = scene->mMeshes[meshIndex];
        if (mesh->mNumBones > 0)
        {
            bHasBones = K_TRUE;
            break;
        }
    }

    std::vector<SModel3DBoneData> boneData = {};
    std::unordered_map<std::string, types::usize> boneIndexData = {};
    std::vector<SModel3DAnimationData> animationData = {};
    if (bHasBones == K_TRUE)
    {
        Print("Info: resource contains bone data");

        Print("Info: creating bone data...");

        usize vertexOffset = 0;
        std::vector<std::vector<SModel3DVertexWeightData>> vertexWeights;
        vertexWeights.resize(vertexCount);
        for (usize meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
        {
            const aiMesh* mesh = scene->mMeshes[meshIndex];
            for (usize boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
            {
                const aiBone* bone = mesh->mBones[boneIndex];

                usize realBoneIndex = 0;
                std::string boneName = bone->mName.C_Str();
                if (boneIndexData.find(boneName) == boneIndexData.end())
                {
                    SModel3DBoneData b = {};
                    b.name = boneName;
                    b.modelMatrix = ConvertMatrix(bone->mOffsetMatrix);

                    realBoneIndex = boneData.size();
                    boneIndexData.insert({ boneName, realBoneIndex });
                    boneData.push_back(std::move(b));
                }
                else
                {
                    realBoneIndex = boneIndexData[boneName];
                }

                for (usize weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++)
                {
                    const aiVertexWeight& vw = bone->mWeights[weightIndex];
                    const usize vertexIndex = vertexOffset + vw.mVertexId;

                    SModel3DVertexWeightData m3dvwd;
                    m3dvwd.boneIndex = realBoneIndex;
                    m3dvwd.weight = vw.mWeight;

                    vertexWeights[vertexIndex].push_back(m3dvwd);
                }
            }

            vertexOffset += mesh->mNumVertices;
        }

        ////////// Finalize vertex weights //////////
        Print("Info: finalizing vertex data weights...");

        for (usize vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
        {
            auto& weights = vertexWeights[vertexIndex];

            std::sort(
                weights.begin(),
                weights.end(),
                [](const SModel3DVertexWeightData& a, const SModel3DVertexWeightData& b)
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

        ////////// Accumulate bone transform //////////
        Print("Info: accumulating bone data transforms...");

        aiNode* boneRootNode = nullptr;
        aiMatrix4x4 parentRootTransform = aiMatrix4x4();
        aiMatrix4x4 accumulatedRootTransform = aiMatrix4x4();
        AccumulateBoneTransform(
            scene->mRootNode,
            boneRootNode,
            parentRootTransform,
            boneIndexData,
            accumulatedRootTransform
        );

        ////////// Create bone hierarchy //////////
        Print("Info: creating bone data hierarchy...");

        ConstructBoneHierarchy(
            boneRootNode,
            -1,
            boneIndexData,
            boneData
        );

        ////////// Create animations //////////
        if (scene->mNumAnimations > 0)
        {
            Print("Info: resource contains animation data");

            Print("Info: creating animation data...");

            for (usize animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
            {
                const aiAnimation* srcAnim = scene->mAnimations[animIdx];

                SModel3DAnimationData animation = {};
                animation.name = srcAnim->mName.C_Str();
                animation.duration = srcAnim->mDuration;
                animation.ticksPerSecond = srcAnim->mTicksPerSecond;
                animation.keys.reserve(srcAnim->mNumChannels);

                for (usize channelIdx = 0; channelIdx < srcAnim->mNumChannels; ++channelIdx)
                {
                    const aiNodeAnim* channel = srcAnim->mChannels[channelIdx];

                    auto it = boneIndexData.find(channel->mNodeName.C_Str());
                    if (it == boneIndexData.end())
                        continue;

                    SModel3DAnimationKeyData key = {};
                    key.localBoneIndex = it->second;

                    // Position
                    key.positionKeys.reserve(channel->mNumPositionKeys);
                    for (usize keyIdx = 0; keyIdx < channel->mNumPositionKeys; ++keyIdx)
                    {
                        const aiVectorKey& assimpKey = channel->mPositionKeys[keyIdx];
                        SModel3DBonePositionKeyData bpk = {};
                        bpk.time = assimpKey.mTime;
                        bpk.position = glm::vec3(assimpKey.mValue.x, assimpKey.mValue.y, assimpKey.mValue.z);
                        key.positionKeys.push_back(bpk);
                    }

                    // Rotation
                    key.rotationKeys.reserve(channel->mNumRotationKeys);
                    for (usize keyIdx = 0; keyIdx < channel->mNumRotationKeys; ++keyIdx)
                    {
                        const aiQuatKey& assimpKey = channel->mRotationKeys[keyIdx];
                        SModel3DBoneRotationKeyData brk = {};
                        brk.time = assimpKey.mTime;
                        brk.rotation = glm::quat(assimpKey.mValue.w, assimpKey.mValue.x, assimpKey.mValue.y, assimpKey.mValue.z);
                        key.rotationKeys.push_back(brk);
                    }

                    // Scale
                    key.scaleKeys.reserve(channel->mNumScalingKeys);
                    for (usize keyIdx = 0; keyIdx < channel->mNumScalingKeys; ++keyIdx)
                    {
                        const aiVectorKey& assimpKey = channel->mScalingKeys[keyIdx];
                        SModel3DBoneScaleKeyData bsk = {};
                        bsk.time = assimpKey.mTime;
                        bsk.scale = glm::vec3(assimpKey.mValue.x, assimpKey.mValue.y, assimpKey.mValue.z);
                        key.scaleKeys.push_back(bsk);
                    }

                    animation.keys.push_back(std::move(key));
                }

                animationData.push_back(animation);
            }
        }
        else
        {
            Print("Info: resource does not contain any animation data");
        }
    }
    else
    {
        Print("Info: resource does not contain any bone data");
    }

    ////////// Return //////////
    Print("Info: preparing result...");

    SModel3DData m3dd;
    m3dd.vertexData = vertexData;
    m3dd.indexData = indexData;
    m3dd.vertexCount = vertexCount;
    m3dd.indexCount = indexCount;
    m3dd.materialData = materialData;
    m3dd.boneData = boneData;
    m3dd.animationData = animationData;

    return m3dd;
}

std::optional<STextureData> ParseModel3DTexture(
    boolean bIsEmbeddedTexture,
    const aiTexture* assimpTexture,
    const std::filesystem::path& modelFolderPath,
    const std::filesystem::path& textureLocalFilePath
)
{
    Print("Info: creating texture from '" + (modelFolderPath / textureLocalFilePath).generic_string() + "'...");

    STextureData td;

    if (bIsEmbeddedTexture == K_TRUE)
    {
        Print("Info: texture is embedded texture");

        ////////// From model byte data //////////
        if (!assimpTexture)
        {
            Print("Error: texture does not exist in given resource");

            return std::nullopt;
        }

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
                if (!pixels)
                {
                    Print("Error: can't read PNG texture");

                    return std::nullopt;
                }
                usize texByteSize = (usize)(width * height * channels);
                u8* texPixels = (u8*)malloc(texByteSize);
                memset(&texPixels[0], texByteSize, 0);
                memcpy(&texPixels[0], &pixels[0], texByteSize);
                stbi_image_free(pixels);

                if (DecidePNGFormat(channels, td.dataFormat) == K_FALSE)
                    return std::nullopt;

                td.format = ETextureFormat::PNG;
                td.width = (usize)width;
                td.height = (usize)height;
                td.channelCount = (usize)channels;
                td.pixelByteData = texPixels;
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
                if (!pixels)
                {
                    Print("Error: can't read DDS texture");

                    return std::nullopt;
                }

                if (DecideDDSFormat(format, td.channelCount, td.dataFormat) == K_FALSE)
                    return std::nullopt;

                channels = td.channelCount;
                usize texByteSize = (usize)(width * height * channels);
                u8* texPixels = (u8*)malloc(texByteSize);
                memset(&texPixels[0], texByteSize, 0);
                memcpy(&texPixels[0], pixels, texByteSize);

                td.format = ETextureFormat::DDS;
                td.width = (usize)width;
                td.height = (usize)height;
                td.pixelByteData = texPixels;
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
        Print("Info: texture is located in separate file");

        ////////// From file //////////
        const std::string textureFilePath = (modelFolderPath / textureLocalFilePath).generic_string();

        if (!std::filesystem::exists(textureFilePath))
        {
            Print("Error: texture file '" + textureFilePath + "' doesn't exist at given location");

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
            if (!pixels)
            {
                Print("Error: can't read PNG texture");

                return std::nullopt;
            }
            usize texByteSize = (usize)(width * height * channels);
            u8* texPixels = (u8*)malloc(texByteSize);
            memset(&texPixels[0], texByteSize, 0);
            memcpy(&texPixels[0], &pixels[0], texByteSize);
            stbi_image_free(pixels);

            if (DecidePNGFormat(channels, td.dataFormat) == K_FALSE)
                return std::nullopt;

            td.format = ETextureFormat::PNG;
            td.width = (usize)width;
            td.height = (usize)height;
            td.channelCount = (usize)channels;
            td.pixelByteData = texPixels;
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
            if (!pixels)
            {
                Print("Error: can't read DDS texture");

                return std::nullopt;
            }

            if (DecideDDSFormat(format, td.channelCount, td.dataFormat) == K_FALSE)
                return std::nullopt;

            channels = td.channelCount;
            usize texByteSize = (usize)(width * height * channels);
            u8* texPixels = (u8*)malloc(texByteSize);
            memset(&texPixels[0], texByteSize, 0);
            memcpy(&texPixels[0], pixels, texByteSize);

            td.format = ETextureFormat::DDS;
            td.width = (usize)width;
            td.height = (usize)height;
            td.pixelByteData = texPixels;
        }
        else
        {
            std::string textureFormatHint = std::string(&assimpTexture->achFormatHint[0]);
            Print("Error: unsupported texture format '" + textureFormatHint + "'");

            return std::nullopt;
        }
    }

    Print("Info: texture was created successfully!");

    return td;
}

boolean DecidePNGFormat(
    usize channels,
    ETextureDataFormat& outDataFormat
)
{
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

    return K_TRUE;
}

boolean DecideDDSFormat(
    tinyddsloader::DDSFile::DXGIFormat format,
    usize& outChannels,
    ETextureDataFormat& outDataFormat
)
{
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

    return K_TRUE;
}

boolean AccumulateBoneTransform(
    const aiNode* root,
    aiNode*& outBoneRoot,
    const aiMatrix4x4& parentTransform,
    const std::unordered_map<std::string, usize>& boneIndexData,
    aiMatrix4x4& outAccumulatedTransform
)
{
    auto it = boneIndexData.find(root->mName.C_Str());
    if (it != boneIndexData.end())
    {
        outBoneRoot = (aiNode*)root;
        outAccumulatedTransform = parentTransform;

        return K_TRUE;
    }

    aiMatrix4x4 currentTransform = parentTransform * root->mTransformation;
    for (usize i = 0; i < root->mNumChildren; i++)
    {
        if (AccumulateBoneTransform(
            root->mChildren[i],
            outBoneRoot,
            currentTransform,
            boneIndexData,
            outAccumulatedTransform
        ))
            return K_TRUE;
    }

    return K_FALSE;
}

void ConstructBoneHierarchy(
    const aiNode* node,
    s32 parentBoneIndex,
    const std::unordered_map<std::string, usize>& boneIndexData,
    std::vector<SModel3DBoneData>& outBoneData
)
{
    s32 currentBoneIndex = -1;

    auto it = boneIndexData.find(node->mName.C_Str());
    if (it != boneIndexData.end())
    {
        currentBoneIndex = it->second;

        outBoneData[currentBoneIndex].parentLocalBoneIndex = parentBoneIndex;
        outBoneData[currentBoneIndex].localMatrix = ConvertMatrix(node->mTransformation);

        if (parentBoneIndex != -1)
            outBoneData[parentBoneIndex].childLocalBoneIndices.push_back(currentBoneIndex);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ConstructBoneHierarchy(
            node->mChildren[i],
            currentBoneIndex == -1 ? parentBoneIndex : currentBoneIndex,
            boneIndexData,
            outBoneData
        );
    }
}

glm::mat4 ConvertMatrix(const aiMatrix4x4& matrix)
{
    glm::mat4 result;
    result[0] = glm::vec4(matrix.a1, matrix.b1, matrix.c1, matrix.d1);
    result[1] = glm::vec4(matrix.a2, matrix.b2, matrix.c2, matrix.d2);
    result[2] = glm::vec4(matrix.a3, matrix.b3, matrix.c3, matrix.d3);
    result[3] = glm::vec4(matrix.a4, matrix.b4, matrix.c4, matrix.d4);

    return result;
}

void ReadFileBytes(
    const std::filesystem::path& filePath,
    u8*& byteData,
    usize& byteSize
)
{
    Print("Info: opening file '" + filePath.generic_string() + "'...");

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        Print("Error: failed to open file");

        return;
    }

    byteSize = (types::usize)file.tellg();

    Print("Info: file byte size = " + std::to_string(byteSize));

    file.seekg(0, std::ios::beg);

    Print("Info: reading file data bytes...");

    byteData = new types::u8[byteSize];
    if (!file.read((char*)byteData, byteSize))
    {
        delete[] byteData;
        byteData = nullptr;
        byteSize = 0;

        Print("Error: failed to read file");

        return;
    }

    Print("Info: file was opened & read successfully!");
}

boolean LoadModel3D(
    std::ifstream& resourceStream,
    SModel3DData& myData
)
{
    ////////// Vertices //////////
    Print("Info: reading vertex data...");

    usize kVertexByteSize = sizeof(SVertex);
    resourceStream.read((char*)&kVertexByteSize, SBaseResourceFileHeader::kUSizeByteSize);
    resourceStream.read((char*)&myData.vertexCount, SBaseResourceFileHeader::kUSizeByteSize);
    myData.vertexData = (SVertex*)malloc(myData.vertexCount * kVertexByteSize);
    for (usize i = 0; i < myData.vertexCount; i++)
        resourceStream.read((char*)&myData.vertexData[i], kVertexByteSize);

    ////////// Indices //////////
    Print("Info: reading index data...");

    usize kIndexByteSize = sizeof(u32);
    resourceStream.read((char*)&kIndexByteSize, SBaseResourceFileHeader::kUSizeByteSize);
    resourceStream.read((char*)&myData.indexCount, SBaseResourceFileHeader::kUSizeByteSize);
    myData.indexData = (u32*)malloc(myData.indexCount * kIndexByteSize);
    for (usize i = 0; i < myData.indexCount; i++)
        resourceStream.read((char*)&myData.indexData[i], kIndexByteSize);

    ////////// Materials //////////
    Print("Info: reading material data...");

    usize materialCount = 0;
    resourceStream.read((char*)&materialCount, SBaseResourceFileHeader::kUSizeByteSize);
    myData.materialData.resize(materialCount);
    for (usize i = 0; i < materialCount; i++)
    {
        SModel3DMaterialData& myMaterialData = myData.materialData[i];

        u8 diffuseTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 normalTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 roughnessTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 metallicTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        resourceStream.read((char*)&diffuseTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.read((char*)&normalTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.read((char*)&roughnessTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.read((char*)&metallicTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        myMaterialData.diffuseTexturePath = std::string((const char*)&diffuseTexturePath[0]);
        myMaterialData.normalTexturePath = std::string((const char*)&normalTexturePath[0]);
        myMaterialData.roughnessTexturePath = std::string((const char*)&roughnessTexturePath[0]);
        myMaterialData.metallicTexturePath = std::string((const char*)&metallicTexturePath[0]);

        auto ReadTextureData = [&](std::ifstream& stream, STextureData& texture, const std::string& label)
            {
                Print("Info: reading " + label + " texture data...");

                stream.read((char*)&texture.format, SBaseResourceFileHeader::kDWordByteSize);
                stream.read((char*)&texture.dataFormat, SBaseResourceFileHeader::kDWordByteSize);
                stream.read((char*)&texture.width, SBaseResourceFileHeader::kUSizeByteSize);
                stream.read((char*)&texture.height, SBaseResourceFileHeader::kUSizeByteSize);
                stream.read((char*)&texture.channelCount, SBaseResourceFileHeader::kUSizeByteSize);
                const usize pixelDataByteSize = texture.width * texture.height * texture.channelCount;
                texture.pixelByteData = (u8*)malloc(pixelDataByteSize);
                for (usize j = 0; j < pixelDataByteSize; j++)
                    stream.read((char*)&texture.pixelByteData[j], SBaseResourceFileHeader::kUCharByteSize);
            };
        ReadTextureData(resourceStream, myMaterialData.diffuseTexture, "diffuse");
        ReadTextureData(resourceStream, myMaterialData.normalTexture, "normal");
        ReadTextureData(resourceStream, myMaterialData.roughnessTexture, "roughness");
        ReadTextureData(resourceStream, myMaterialData.metallicTexture, "metallic");
    }

    ////////// Bones & Animations //////////
    usize boneCount = 0;
    resourceStream.read((char*)&boneCount, SBaseResourceFileHeader::kUSizeByteSize);
    if (boneCount > 0)
    {
        Print("Info: reading bone data...");

        myData.boneData.resize(boneCount);

        ////////// Bones //////////
        for (usize i = 0; i < boneCount; i++)
        {
            SModel3DBoneData& myBoneData = myData.boneData[i];

            u8 boneName[SBaseResourceFileHeader::kStringByteSize] = {};
            resourceStream.read((char*)&boneName[0], SBaseResourceFileHeader::kStringByteSize);
            myBoneData.name = std::string((const char*)&boneName[0]);

            resourceStream.read((char*)&myBoneData.localMatrix[0][0], SBaseResourceFileHeader::kMatrixByteSize);
            resourceStream.read((char*)&myBoneData.modelMatrix[0][0], SBaseResourceFileHeader::kMatrixByteSize);
            resourceStream.read((char*)&myBoneData.parentLocalBoneIndex, SBaseResourceFileHeader::kSIntByteSize);

            usize childBoneCount = (usize)myBoneData.childLocalBoneIndices.size();
            resourceStream.read((char*)&childBoneCount, SBaseResourceFileHeader::kUSizeByteSize);
            myBoneData.childLocalBoneIndices.resize(childBoneCount);
            for (usize j = 0; j < childBoneCount; j++)
                resourceStream.read((char*)&myBoneData.childLocalBoneIndices[j], SBaseResourceFileHeader::kUSizeByteSize);
        }

        ////////// Animations //////////
        Print("Info: reading animation data...");

        usize animationCount = 0;
        resourceStream.read((char*)&animationCount, SBaseResourceFileHeader::kUSizeByteSize);
        myData.animationData.resize(animationCount);
        for (usize i = 0; i < animationCount; i++)
        {
            SModel3DAnimationData& myAnimationData = myData.animationData[i];

            u8 animationName[SBaseResourceFileHeader::kStringByteSize] = {};
            resourceStream.read((char*)&animationName[0], SBaseResourceFileHeader::kStringByteSize);
            myAnimationData.name = std::string((const char*)&animationName[0]);

            resourceStream.read((char*)&myAnimationData.duration, SBaseResourceFileHeader::kFloatByteSize);
            resourceStream.read((char*)&myAnimationData.ticksPerSecond, SBaseResourceFileHeader::kFloatByteSize);

            Print("Info: reading key data...");

            usize keyCount = myAnimationData.keys.size();
            resourceStream.read((char*)&keyCount, SBaseResourceFileHeader::kUSizeByteSize);
            myAnimationData.keys.resize(keyCount);
            for (usize j = 0; j < keyCount; j++)
            {
                SModel3DAnimationKeyData& myAnimationKeyData = myAnimationData.keys[j];

                resourceStream.read((char*)&myAnimationKeyData.localBoneIndex, SBaseResourceFileHeader::kUSizeByteSize);

                Print("Info: reading position key data...");

                usize positionKeyCount = 0;
                resourceStream.read((char*)&positionKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                myAnimationKeyData.positionKeys.resize(positionKeyCount);
                for (usize k = 0; k < positionKeyCount; k++)
                {
                    SModel3DBonePositionKeyData& myBonePositionKeyData = myAnimationKeyData.positionKeys[k];
                    resourceStream.read((char*)&myBonePositionKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.read((char*)&myBonePositionKeyData.position, SBaseResourceFileHeader::kVectorByteSize);
                }

                Print("Info: reading rotation key data...");

                usize rotationKeyCount = myAnimationKeyData.rotationKeys.size();
                resourceStream.read((char*)&rotationKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                myAnimationKeyData.rotationKeys.resize(rotationKeyCount);
                for (usize k = 0; k < rotationKeyCount; k++)
                {
                    SModel3DBoneRotationKeyData& myBoneRotationKeyData = myAnimationKeyData.rotationKeys[k];
                    resourceStream.read((char*)&myBoneRotationKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.read((char*)&myBoneRotationKeyData.rotation, SBaseResourceFileHeader::kQuaternionByteSize);
                }

                Print("Info: reading scale key data...");

                usize scaleKeyCount = myAnimationKeyData.scaleKeys.size();
                resourceStream.read((char*)&scaleKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                myAnimationKeyData.scaleKeys.resize(scaleKeyCount);
                for (usize k = 0; k < scaleKeyCount; k++)
                {
                    SModel3DBoneScaleKeyData& myBoneScaleKeyData = myAnimationKeyData.scaleKeys[k];
                    resourceStream.read((char*)&myBoneScaleKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.read((char*)&myBoneScaleKeyData.scale, SBaseResourceFileHeader::kVectorByteSize);
                }
            }
        }
    }
    else
    {
        Print("Info: no any bone data to read");
    }

    return K_TRUE;
}

boolean WriteModel3D(
    std::ofstream& resourceStream,
    const SModel3DData& myData
)
{
    const usize kStringByteSizeWithoutZero = SBaseResourceFileHeader::kStringByteSize - 1;

    ////////// Vertices //////////
    Print("Info: writing vertex data...");

    const usize kVertexByteSize = sizeof(SVertex);
    resourceStream.write((const char*)&kVertexByteSize, SBaseResourceFileHeader::kUSizeByteSize);
    resourceStream.write((const char*)&myData.vertexCount, SBaseResourceFileHeader::kUSizeByteSize);
    for (usize i = 0; i < myData.vertexCount; i++)
        resourceStream.write((const char*)&myData.vertexData[i], kVertexByteSize);

    ////////// Indices //////////
    Print("Info: writing index data...");

    const usize kIndexByteSize = sizeof(u32);
    resourceStream.write((const char*)&kIndexByteSize, SBaseResourceFileHeader::kUSizeByteSize);
    resourceStream.write((const char*)&myData.indexCount, SBaseResourceFileHeader::kUSizeByteSize);
    for (usize i = 0; i < myData.indexCount; i++)
        resourceStream.write((const char*)&myData.indexData[i], kIndexByteSize);

    ////////// Materials //////////
    Print("Info: writing material data...");

    const usize materialCount = (usize)myData.materialData.size();
    resourceStream.write((const char*)&materialCount, SBaseResourceFileHeader::kUSizeByteSize);
    for (usize i = 0; i < materialCount; i++)
    {
        const SModel3DMaterialData& myMaterialData = myData.materialData[i];

        if (myMaterialData.diffuseTexturePath.size() > kStringByteSizeWithoutZero ||
            myMaterialData.normalTexturePath.size() > kStringByteSizeWithoutZero ||
            myMaterialData.roughnessTexturePath.size() > kStringByteSizeWithoutZero ||
            myMaterialData.metallicTexturePath.size() > kStringByteSizeWithoutZero)
        {
            Print("Error: texture path exceeds string byte size = " + std::to_string(kStringByteSizeWithoutZero));
            return K_FALSE;
        }

        u8 diffuseTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 normalTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 roughnessTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        u8 metallicTexturePath[SBaseResourceFileHeader::kStringByteSize] = {};
        memcpy(
            &diffuseTexturePath[0],
            &myMaterialData.diffuseTexturePath.c_str()[0],
            myMaterialData.diffuseTexturePath.size()
        );
        memcpy(
            &normalTexturePath[0],
            &myMaterialData.normalTexturePath.c_str()[0],
            myMaterialData.normalTexturePath.size()
        );
        memcpy(
            &roughnessTexturePath[0],
            &myMaterialData.roughnessTexturePath.c_str()[0],
            myMaterialData.roughnessTexturePath.size()
        );
        memcpy(
            &metallicTexturePath[0],
            &myMaterialData.metallicTexturePath.c_str()[0],
            myMaterialData.metallicTexturePath.size()
        );

        resourceStream.write((const char*)&diffuseTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.write((const char*)&normalTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.write((const char*)&roughnessTexturePath[0], SBaseResourceFileHeader::kStringByteSize);
        resourceStream.write((const char*)&metallicTexturePath[0], SBaseResourceFileHeader::kStringByteSize);

        auto WriteTextureData = [](std::ofstream& stream, const STextureData& texture, const std::string& label)
        {
            Print("Info: writing " + label + " texture data...");

            stream.write((const char*)&texture.format, SBaseResourceFileHeader::kDWordByteSize);
            stream.write((const char*)&texture.dataFormat, SBaseResourceFileHeader::kDWordByteSize);
            stream.write((const char*)&texture.width, SBaseResourceFileHeader::kUSizeByteSize);
            stream.write((const char*)&texture.height, SBaseResourceFileHeader::kUSizeByteSize);
            stream.write((const char*)&texture.channelCount, SBaseResourceFileHeader::kUSizeByteSize);
            const usize pixelDataByteSize = texture.width * texture.height * texture.channelCount;
            for (usize j = 0; j < pixelDataByteSize; j++)
                stream.write((const char*)&texture.pixelByteData[j], SBaseResourceFileHeader::kUCharByteSize);
        };
        WriteTextureData(resourceStream, myMaterialData.diffuseTexture, "diffuse");
        WriteTextureData(resourceStream, myMaterialData.normalTexture, "normal");
        WriteTextureData(resourceStream, myMaterialData.roughnessTexture, "roughness");
        WriteTextureData(resourceStream, myMaterialData.metallicTexture, "metallic");
    }

    ////////// Bones & Animations //////////
    const usize boneCount = (usize)myData.boneData.size();
    resourceStream.write((const char*)&boneCount, SBaseResourceFileHeader::kUSizeByteSize);
    if (boneCount > 0)
    {
        Print("Info: writing bone data...");

        ////////// Bones //////////
        for (usize i = 0; i < boneCount; i++)
        {
            const SModel3DBoneData& myBoneData = myData.boneData[i];

            if (myBoneData.name.size() > kStringByteSizeWithoutZero)
            {
                Print("Error: bone name exceeds string byte size = " + std::to_string(kStringByteSizeWithoutZero));
                return K_FALSE;
            }
            u8 boneName[SBaseResourceFileHeader::kStringByteSize] = {};
            memcpy(
                &boneName[0],
                &myBoneData.name.c_str()[0],
                myBoneData.name.size()
            );
            resourceStream.write((const char*)&boneName[0], SBaseResourceFileHeader::kStringByteSize);

            resourceStream.write((const char*)&myBoneData.localMatrix[0][0], SBaseResourceFileHeader::kMatrixByteSize);
            resourceStream.write((const char*)&myBoneData.modelMatrix[0][0], SBaseResourceFileHeader::kMatrixByteSize);
            resourceStream.write((const char*)&myBoneData.parentLocalBoneIndex, SBaseResourceFileHeader::kSIntByteSize);

            const usize childBoneCount = (usize)myBoneData.childLocalBoneIndices.size();
            resourceStream.write((const char*)&childBoneCount, SBaseResourceFileHeader::kUSizeByteSize);
            for (usize j = 0; j < childBoneCount; j++)
                resourceStream.write((const char*)&myBoneData.childLocalBoneIndices[j], SBaseResourceFileHeader::kUSizeByteSize);
        }

        ////////// Animations //////////
        Print("Info: writing animation data...");

        const usize animationCount = myData.animationData.size();
        resourceStream.write((const char*)&animationCount, SBaseResourceFileHeader::kUSizeByteSize);
        for (usize i = 0; i < animationCount; i++)
        {
            const SModel3DAnimationData& myAnimationData = myData.animationData[i];

            if (myAnimationData.name.size() > kStringByteSizeWithoutZero)
            {
                Print("Error: animation name exceeds string byte size = " + std::to_string(kStringByteSizeWithoutZero));
                return K_FALSE;
            }
            u8 animationName[SBaseResourceFileHeader::kStringByteSize] = {};
            memcpy(
                &animationName[0],
                &myAnimationData.name.c_str()[0],
                myAnimationData.name.size()
            );
            resourceStream.write((const char*)&animationName[0], SBaseResourceFileHeader::kStringByteSize);

            resourceStream.write((const char*)&myAnimationData.duration, SBaseResourceFileHeader::kFloatByteSize);
            resourceStream.write((const char*)&myAnimationData.ticksPerSecond, SBaseResourceFileHeader::kFloatByteSize);

            Print("Info: writing key data...");

            const usize keyCount = myAnimationData.keys.size();
            resourceStream.write((const char*)&keyCount, SBaseResourceFileHeader::kUSizeByteSize);
            for (usize j = 0; j < keyCount; j++)
            {
                const SModel3DAnimationKeyData& myAnimationKeyData = myAnimationData.keys[j];

                resourceStream.write((const char*)&myAnimationKeyData.localBoneIndex, SBaseResourceFileHeader::kUSizeByteSize);

                Print("Info: writing position key data...");

                const usize positionKeyCount = myAnimationKeyData.positionKeys.size();
                resourceStream.write((const char*)&positionKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                for (usize k = 0; k < positionKeyCount; k++)
                {
                    const SModel3DBonePositionKeyData& myBonePositionKeyData = myAnimationKeyData.positionKeys[k];
                    resourceStream.write((const char*)&myBonePositionKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.write((const char*)&myBonePositionKeyData.position, SBaseResourceFileHeader::kVectorByteSize);
                }

                Print("Info: writing rotation key data...");

                const usize rotationKeyCount = myAnimationKeyData.rotationKeys.size();
                resourceStream.write((const char*)&rotationKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                for (usize k = 0; k < rotationKeyCount; k++)
                {
                    const SModel3DBoneRotationKeyData& myBoneRotationKeyData = myAnimationKeyData.rotationKeys[k];
                    resourceStream.write((const char*)&myBoneRotationKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.write((const char*)&myBoneRotationKeyData.rotation, SBaseResourceFileHeader::kQuaternionByteSize);
                }

                Print("Info: writing scale key data...");

                const usize scaleKeyCount = myAnimationKeyData.scaleKeys.size();
                resourceStream.write((const char*)&scaleKeyCount, SBaseResourceFileHeader::kUSizeByteSize);
                for (usize k = 0; k < scaleKeyCount; k++)
                {
                    const SModel3DBoneScaleKeyData& myBoneScaleKeyData = myAnimationKeyData.scaleKeys[k];
                    resourceStream.write((const char*)&myBoneScaleKeyData.time, SBaseResourceFileHeader::kFloatByteSize);
                    resourceStream.write((const char*)&myBoneScaleKeyData.scale, SBaseResourceFileHeader::kVectorByteSize);
                }
            }
        }
    }
    else
    {
        Print("Info: no any bone data to write");
    }

    return K_TRUE;
}

boolean CModel3DAsset::LoadRawFile(const std::filesystem::path& rawFilePath)
{
    auto timeStart = std::chrono::high_resolution_clock::now();

    boolean bIsOk = K_FALSE;

    ReadFileBytes(
        rawFilePath,
        data,
        dataByteSize
    );

    const std::string rawFileDirPath = rawFilePath.parent_path().generic_string();
    const std::string rawFileLocalPath = rawFilePath.filename().generic_string();
    auto result = ParseModel3D(
        data,
        dataByteSize,
        rawFileDirPath,
        rawFileLocalPath
    );

    if (result.has_value())
        bIsOk = K_TRUE;

    if (bIsOk == K_TRUE)
    {
        *((SModel3DData*)this) = *result;

        Print("Info: resource was parsed successfully!");
    }
    else
    {
        Print("Error: failed to parse resource!");
        return K_FALSE;
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    f64 seconds = std::chrono::duration<f64>(timeEnd - timeStart).count();

    Print("Info: elapsed time = " + std::to_string(seconds) + " sec");

    return K_TRUE;
}

boolean CModel3DAsset::LoadAssetFile(const std::filesystem::path& assetFilePath)
{
    auto timeStart = std::chrono::high_resolution_clock::now();
    boolean bIsOk = K_FALSE;

    const std::string& resourceFilePathStr = assetFilePath.generic_string();

    Print(
        "Info: reading 3d model resource at '" +
        resourceFilePathStr +
        "'..."
    );

    Print("Info: opening file '" + resourceFilePathStr + "'...");

    std::ifstream resourceStream(assetFilePath.generic_string().c_str(), std::ios::binary);
    if (!resourceStream)
    {
        Print("Error: failed to open file '" + assetFilePath.generic_string() + "'");

        return K_FALSE;
    }

    ////////// Header //////////
    Print("Info: reading header...");

    dword headerMagic = 0;
    usize headerNumByteSize = 0;
    usize headerUCharByteSize = 0;
    usize headerSIntByteSize = 0;
    usize headerUIntByteSize = 0;
    usize headerUSizeByteSize = 0;
    usize headerDWordByteSize = 0;
    usize headerQWordByteSize = 0;
    usize headerFloatByteSize = 0;
    usize headerVectorByteSize = 0;
    usize headerQuaternionByteSize = 0;
    usize headerMatrixByteSize = 0;
    usize headerStringByteSize = 0;
    EAssetFormat headerFormat = EAssetFormat::Unknown;
    qword headerReserved = 0;
    resourceStream.read((char*)&headerMagic, kMagicByteCount);
    resourceStream.read((char*)&headerUCharByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerSIntByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerUIntByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerUSizeByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerDWordByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerQWordByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerFloatByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerVectorByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerQuaternionByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerMatrixByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerStringByteSize, kHeaderNumByteSize);
    resourceStream.read((char*)&headerFormat, headerDWordByteSize);
    resourceStream.read((char*)&headerReserved, headerQWordByteSize);

    if (headerFormat != EAssetFormat::Model3D)
    {
        Print("Error: resource format mismatch");
        return K_FALSE;
    }

    SModel3DData& myData = *((SModel3DData*)this);

    boolean result = LoadModel3D(
        resourceStream,
        myData
    );

    resourceStream.close();

    if (result == K_FALSE)
    {
        Print("Info: failed to read resource");
    }
    else
    {
        Print("Info: resource file was read successfully!");
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    f64 seconds = std::chrono::duration<f64>(timeEnd - timeStart).count();

    Print("Info: elapsed time = " + std::to_string(seconds) + " sec");

    return result;
}

boolean CModel3DAsset::WriteAssetFile(const std::filesystem::path& assetFilePath)
{
    auto timeStart = std::chrono::high_resolution_clock::now();
    boolean bIsOk = K_FALSE;

    const std::string& resourceFilePathStr = assetFilePath.generic_string();

    Print(
        "Info: writing 3d model resource at '" +
        resourceFilePathStr +
        "'..."
    );

    if (kStringByteSize <= 1)
    {
        Print("Error: string byte size is too short = " + std::to_string(kStringByteSize));
        return K_FALSE;
    }
    const usize kStringByteSizeWithoutZero = kStringByteSize - 1;

    Print("Info: opening file '" + resourceFilePathStr + "'...");

    std::ofstream resourceStream(assetFilePath.generic_string().c_str(), std::ios::binary);
    if (!resourceStream)
    {
        Print("Error: failed to open file '" + assetFilePath.generic_string() + "'");

        return K_FALSE;
    }

    ////////// Header //////////
    Print("Info: writing header...");

    format = EAssetFormat::Model3D;
    reserved = 0;
    resourceStream.write((const char*)&kMagic[0], kMagicByteCount);
    resourceStream.write((const char*)&kUCharByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kSIntByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kUIntByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kUSizeByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kDWordByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kQWordByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kFloatByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kVectorByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kQuaternionByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kMatrixByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&kStringByteSize, kHeaderNumByteSize);
    resourceStream.write((const char*)&format, kDWordByteSize);
    resourceStream.write((const char*)&reserved, kQWordByteSize);

    const SModel3DData& myData = (const SModel3DData&)*this;
    
    boolean result = WriteModel3D(
        resourceStream,
        myData
    );

    resourceStream.close();

    if (result == K_FALSE)
    {
        Print("Info: failed to write resource");
    }
    else
    {
        Print("Info: resource file was written successfully!");
    }

    auto timeEnd = std::chrono::high_resolution_clock::now();
    f64 seconds = std::chrono::duration<f64>(timeEnd - timeStart).count();

    Print("Info: elapsed time = " + std::to_string(seconds) + " sec");

    return result;
}

boolean CModel3DAsset::Destroy()
{
    return K_FALSE;
}