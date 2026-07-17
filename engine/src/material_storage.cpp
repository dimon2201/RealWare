// material_storage.cpp

#include "material_storage.hpp"

triton::HMaterial triton::XMaterialStorage::CreateMaterial(
	const cVector4& diffuseColor,
	const HTexture& diffuseTexture,
	const HTexture& normalTexture,
	const HTexture& roughnessTexture,
	const HTexture& metallicTexture
)
{
    HMaterial material = Create();

    SMaterial materialData;
    materialData.diffuseColor = diffuseColor;
    materialData.diffuseTexture = diffuseTexture;
    materialData.normalTexture = normalTexture;
    materialData.roughnessTexture = roughnessTexture;
    materialData.metallicTexture = metallicTexture;

    Get(material) = materialData;

	return material;
}

void triton::XMaterialStorage::DestroyMaterial(const HMaterial& material)
{
    Destroy(material);
}