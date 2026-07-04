// model3d_backend_assimp.cpp

#include <filesystem>
#include "model3d_backend_assimp.hpp"
#include "model3d_backend_resource.hpp"
#include "context.hpp"
#include "vertex.hpp"
#include "math.hpp"
#include "material_subsystem.hpp"

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
        modelMaterials
    );
    SetAbsoluteMaterialIndices(vertexData, vertexCount, modelMaterials);

    ParseIndexData(scene, indexData, indexOffsets);

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
        aiProcess_RemoveComponent |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
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

        SModel3DMaterialData m3dmd;
        m3dmd.diffuseTextureFilePath = diffuseTexturePath.C_Str();
        m3dmd.normalTextureFilePath = normalTexturePath.C_Str();
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

void triton::XModel3DBackendAssimp::CreateMaterials(const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, XMaterialSubsystem* materialSubsystem, const std::vector<SModel3DMaterialData>& materials, std::vector<HMaterial>& modelMaterials)
{
    for (auto& material : materials)
    {
        HTexture diffuseTexture = *CreateTextureFromFile(modelFolderPath, textureSubsystem, material.diffuseTextureFilePath);
        HTexture normalTexture = *CreateTextureFromFile(modelFolderPath, textureSubsystem, material.normalTextureFilePath);

        modelMaterials.push_back(materialSubsystem->CreateMaterial(cVector4(1.0f), diffuseTexture, normalTexture));
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

std::optional<triton::HTexture> triton::XModel3DBackendAssimp::CreateTextureFromFile(const std::string& modelFolderPath, XTextureSubsystem* textureSubsystem, const std::string& textureFilePath)
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
    
    return textureSubsystem->CreateTexture(newTextureFilePath);
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