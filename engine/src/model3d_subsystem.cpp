// model3d_subsystem.cpp

#include "model3d_subsystem.hpp"
#include "model3d_backend.hpp"

std::optional<triton::HModel3D> triton::XModel3DSubsystem::CreateModel(const std::string& modelFolderPath, const std::string& modelLocalPath)
{
	auto result = _context->GetBackend<IModel3DBackend>()->CreateModel(modelFolderPath, modelLocalPath);
	if (!result)
		return std::nullopt;

	SModel3DBackendResource mbr = *result;
	HModel3D model = Create();
	Get(model).resource = mbr;

	return model;
}

void triton::XModel3DSubsystem::DestroyModel(const HModel3D& model)
{
	SModel3DData& modelData = Get(model);
	_context->GetBackend<IModel3DBackend>()->DestroyModel(modelData.resource);
	Destroy(model);
}