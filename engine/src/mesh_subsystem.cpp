// mesh_subsystem.cpp

#include "mesh_subsystem.hpp"
#include "mesh_backend.hpp"

std::optional<triton::HMesh> triton::XMeshSubsystem::CreateMesh(const std::string& filePath)
{
	auto result = _context->GetBackend<IMeshBackend>()->CreateMesh(filePath);
	if (!result)
		return std::nullopt;

	SMeshBackendResource mbr = *result;
	HMesh mesh = Create();
	Get(mesh).resource = mbr;

	return mesh;
}

void triton::XMeshSubsystem::DestroyMesh(const HMesh& mesh)
{
	SMeshData& meshData = Get(mesh);
	_context->GetBackend<IMeshBackend>()->DestroyMesh(meshData.resource);
	Destroy(mesh);
}