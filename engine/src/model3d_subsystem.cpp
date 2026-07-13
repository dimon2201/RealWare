// model3d_subsystem.cpp

#include "model3d_subsystem.hpp"
#include "model3d_backend.hpp"

std::optional<triton::HModel3D> triton::XModel3DSubsystem::CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath)
{
	auto result = _context->GetBackend<IModel3DBackend>()->CreateModel(modelFolderPath, modelLocalPath);
	if (!result)
		return std::nullopt;

	SModel3DData& m3ddr = *result;
	HModel3D model = Create();
	SModel3DData& m3ddl = Get(model);
	m3ddl.animations = m3ddr.animations;
	m3ddl.indexCount = m3ddr.indexCount;
	m3ddl.indexData = m3ddr.indexData;
	m3ddl.materials = m3ddr.materials;
	m3ddl.vertexCount = m3ddr.vertexCount;
	m3ddl.vertexData = m3ddr.vertexData;

	return model;
}

void triton::XModel3DSubsystem::DestroyModel(const HModel3D& model)
{
	SModel3DData& modelData = Get(model);
	_context->GetBackend<IModel3DBackend>()->DestroyModel(modelData);
	Destroy(model);
}