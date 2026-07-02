// mesh_backend_assimp.cpp

#include "mesh_backend_assimp.hpp"
#include "mesh_backend_resource.hpp"
#include "context.hpp"
#include "vertex.hpp"
#include "math.hpp"

using namespace types;

std::optional<triton::SMeshBackendResource> triton::CMeshBackendAssimp::CreateMesh(const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    ImportScene(importer, scene, filePath);
    if (!scene)
        return std::nullopt;
    
    SVertex* vertexData = nullptr;
    u32* indexData = nullptr;
    cVector3* bitangents = nullptr;
    usize vertexCount = 0;
    usize indexCount = 0;
    std::vector<usize> indexOffsets = {};
    CountVerticesIndices(scene, vertexCount, indexCount, indexOffsets);
    AllocateVertexIndexBuffers(vertexData, indexData, vertexCount, indexCount);
    AllocateTempBitangentBuffer(bitangents, vertexCount);
    ParseVertexData(scene, vertexData, bitangents);
    CalculateHandedness(vertexData, bitangents, vertexCount);
    DeallocateTempBitangentBuffer(bitangents);
    ParseIndexData(scene, indexData, indexOffsets);

    return PrepareResult(vertexData, indexData, vertexCount, indexCount);
}

void triton::CMeshBackendAssimp::DestroyMesh(SMeshBackendResource& mesh)
{
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
    if (mesh.indexData)
        _context->GetMemoryAllocator()->Deallocate((void*)mesh.indexData);
    if (mesh.vertexData)
        _context->GetMemoryAllocator()->Deallocate((void*)mesh.vertexData);
}

void triton::CMeshBackendAssimp::ImportScene(Assimp::Importer& importer, const aiScene*& scene, const std::string& filePath)
{
    scene = importer.ReadFile(
        filePath.c_str(),
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_GenNormals |
        aiProcess_ImproveCacheLocality
    );
    if (!scene)
        Print("Error: can't load mesh from file '" + filePath + "'");
}

void triton::CMeshBackendAssimp::CountVerticesIndices(const aiScene* scene, usize& vertexCount, usize& indexCount, std::vector<usize>& indexOffsets)
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

void triton::CMeshBackendAssimp::AllocateVertexIndexBuffers(SVertex*& vertexData, u32*& indices, usize vertexCount, usize indexCount)
{
    vertexData = (SVertex*)_context->GetMemoryAllocator()->Allocate(vertexCount * sizeof(SVertex), 64);
    indices = (u32*)_context->GetMemoryAllocator()->Allocate(indexCount * sizeof(u32), 64);
}

void triton::CMeshBackendAssimp::AllocateTempBitangentBuffer(cVector3*& bitangents, usize vertexCount)
{
    bitangents = (cVector3*)_context->GetMemoryAllocator()->Allocate(vertexCount * sizeof(cVector3), 64);
}

void triton::CMeshBackendAssimp::ParseVertexData(const aiScene* scene, SVertex* vertexData, cVector3* bitangents)
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
            
            vertexData[globalVertexIndex].position = cVector3(position.x, position.y, position.z);
            vertexData[globalVertexIndex].texcoord = cVector2(texcoord.x, texcoord.y);
            vertexData[globalVertexIndex].normal = cVector3(normal.x, normal.y, normal.z);
            vertexData[globalVertexIndex].tangent = cVector4(tangent.x, tangent.y, tangent.z, 0.0f);
            bitangents[globalVertexIndex] = cVector3(bitangent.x, bitangent.y, bitangent.z);
            
            globalVertexIndex += 1;
        }
    }
}

void triton::CMeshBackendAssimp::CalculateHandedness(SVertex* vertexData, cVector3* bitangents, usize vertexCount)
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

void triton::CMeshBackendAssimp::ParseIndexData(const aiScene* scene, u32* indexData, const std::vector<usize>& indexOffsets)
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

void triton::CMeshBackendAssimp::DeallocateTempBitangentBuffer(cVector3* bitangents)
{
    _context->GetMemoryAllocator()->Deallocate(bitangents);
}

triton::SMeshBackendResource triton::CMeshBackendAssimp::PrepareResult(const SVertex* vertexData, const u32* indexData, usize vertexCount, usize indexCount)
{
    SMeshBackendResource mbr;
    mbr.vertexData = vertexData;
    mbr.indexData = indexData;
    mbr.vertexCount = vertexCount;
    mbr.indexCount = indexCount;

    return mbr;
}