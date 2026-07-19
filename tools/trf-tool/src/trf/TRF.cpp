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
#include "TRF.hpp"

using namespace triton::resource_file;
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

types::boolean ParsePNG(
    types::usize width,
    types::usize height,
    types::usize channels,
    types::u8* pixels,
    ETextureFormat& outFormat,
    ETextureDataFormat& outDataFormat,
    types::usize& outWidth,
    types::usize& outHeight,
    types::usize& outChannels,
    types::u8*& outPixels
);

types::boolean ParseDDS(
    types::usize width,
    types::usize height,
    tinyddsloader::DDSFile::DXGIFormat format,
    types::u8* pixels,
    ETextureFormat& outFormat,
    ETextureDataFormat& outDataFormat,
    types::usize& outWidth,
    types::usize& outHeight,
    types::usize& outChannels,
    types::u8*& outPixels
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

void triton::resource_file::Print(const std::string& message)
{
    std::cout << message << std::endl;
}

template <EResourceFormat TResourceFormat>
CResourceFile<TResourceFormat>::CResourceFile(const std::filesystem::path& filePath)
{
    Print("Info: opening file '" + filePath.generic_string() + "'...");

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        Print("Error: failed to open file");

        return;
    }

    _dataFolderPath = filePath.parent_path().generic_string();
    _dataLocalFilePath = filePath.filename().generic_string();
    _dataByteSize = (types::usize)file.tellg();

    Print("Info: file byte size = " + std::to_string(_dataByteSize));

    file.seekg(0, std::ios::beg);

    Print("Info: reading file data bytes...");

    _data = new types::u8[_dataByteSize];
    if (!file.read((char*)_data, _dataByteSize))
    {
        delete[] _data;
        _data = nullptr;
        _dataByteSize = 0;

        Print("Error: failed to read file");

        return;
    }

    Print("Info: file was opened & read successfully!");
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
        ParseModel3D(
            _data,
            _dataByteSize,
            _dataFolderPath,
            _dataLocalFilePath
        );
        break;
    }
    }

    Print("Info: resource was parsed successfully!");
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
        "'"
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

    Print("Info: texture was created successfully!");

    return td;
}

boolean ParsePNG(
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

boolean ParseDDS(
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

template class CResourceFile<EResourceFormat::Model3D>;