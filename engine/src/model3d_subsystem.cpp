// model3d_subsystem.cpp

#include "model3d_subsystem.hpp"
#include "model3d_backend.hpp"
#include "tasset.hpp"
#include "material_subsystem.hpp"
#include "animation_subsystem.hpp"
#include "skeleton_subsystem.hpp"

using namespace types;

std::optional<triton::HModel3D> triton::XModel3DSubsystem::CreateModelFromRaw(const std::filesystem::path& modelFilePath)
{
	std::filesystem::path modelFolderPath = modelFilePath.parent_path();
	std::filesystem::path modelLocalPath = modelFilePath.filename();

	auto result = _context->GetBackend<IModel3DBackend>()->CreateModel(
		modelFolderPath.generic_string(),
		modelLocalPath.generic_string()
	);
	if (!result)
		return std::nullopt;

	SModel3DData& m3ddr = *result;
	HModel3D model = Create();
	SModel3DData& m3ddl = Get(model);
	m3ddl = m3ddr;

	return model;
}

std::optional<triton::HModel3D> triton::XModel3DSubsystem::CreateModelFromAsset(const std::filesystem::path& assetFilePath)
{
	std::filesystem::path assetFolderPath = assetFilePath.parent_path();

	asset::CModel3DAsset asset;
	asset.LoadAssetFile(assetFilePath);

	HModel3D model = Create();
	SModel3DData& m3dd = Get(model);

	m3dd.vertexData = (const SVertex*)asset.vertexData;
	m3dd.vertexCount = asset.vertexCount;
	m3dd.indexData = asset.indexData;
	m3dd.indexCount = asset.indexCount;

	for (usize i = 0; i < asset.materialData.size(); i++)
	{
		asset::SModel3DMaterialData& assetMaterialData = asset.materialData[i];

		auto ProcessTexture = [](
			const std::filesystem::path& assetFolderPath,
			const std::filesystem::path& assetTexturePath,
			const asset::STextureData& assetTextureData,
			cTexture::eFormat textureFormat,
			XTextureSubsystem* textureSubsystem,
			HTexture& outHandle
		) {
			std::filesystem::path texturePath = assetTexturePath;
			if (!assetTexturePath.is_absolute())
				texturePath = assetFolderPath / assetTexturePath;
			if (std::filesystem::exists(texturePath))
				outHandle = textureSubsystem->CreateTexture(
					texturePath.generic_string(),
					textureFormat
				);
			else
				outHandle = textureSubsystem->CreateTexture(
					assetTextureData.pixelByteData,
					assetTextureData.width * assetTextureData.height * assetTextureData.channelCount,
					assetTextureData.width,
					assetTextureData.height,
					assetTextureData.channelCount,
					ETextureFormat::Raw,
					textureFormat
				);
		};

		HTexture diffuseTexture;
		HTexture normalTexture;
		HTexture roughnessTexture;
		HTexture metallicTexture;
		XTextureSubsystem* textureSubsystem = _context->GetSubsystem<XTextureSubsystem>();

		ProcessTexture(
			assetFolderPath,
			std::filesystem::path(assetMaterialData.diffuseTexturePath),
			assetMaterialData.diffuseTexture,
			cTexture::eFormat::RGBA8_SRGB_MIPS,
			textureSubsystem,
			diffuseTexture
		);
		ProcessTexture(
			assetFolderPath,
			std::filesystem::path(assetMaterialData.normalTexturePath),
			assetMaterialData.normalTexture,
			cTexture::eFormat::RGBA8,
			textureSubsystem,
			normalTexture
		);
		ProcessTexture(
			assetFolderPath,
			std::filesystem::path(assetMaterialData.roughnessTexturePath),
			assetMaterialData.roughnessTexture,
			cTexture::eFormat::R8,
			textureSubsystem,
			roughnessTexture
		);
		ProcessTexture(
			assetFolderPath,
			std::filesystem::path(assetMaterialData.metallicTexturePath),
			assetMaterialData.metallicTexture,
			cTexture::eFormat::R8,
			textureSubsystem,
			metallicTexture
		);

		HMaterial material = _context->GetSubsystem<XMaterialSubsystem>()->CreateMaterial(
			cVector4(1.0f),
			diffuseTexture,
			normalTexture,
			roughnessTexture,
			metallicTexture
		);

		m3dd.materials.push_back(material);
	}

	std::vector<SBone> bones;
	bones.resize(asset.boneData.size());
	for (usize i = 0; i < asset.boneData.size(); i++)
	{
		bones[i].name = asset.boneData[i].name;
		bones[i].localMatrix = cMatrix4(asset.boneData[i].localMatrix);
		bones[i].modelMatrix = cMatrix4(asset.boneData[i].modelMatrix);
		bones[i].localParentBoneIndex = asset.boneData[i].parentLocalBoneIndex;
		bones[i].localChildBoneIndices = asset.boneData[i].childLocalBoneIndices;
	}

	if (bones.size() > 0)
	{
		m3dd.skeleton = _context->GetSubsystem<XSkeletonSubsystem>()->CreateSkeleton(
			bones,
			cMatrix4(asset.rootBoneAccumulatedTransform)
		);
	}
	else
	{
		m3dd.skeleton.Invalidate();
	}
	
	for (usize i = 0; i < asset.animationData.size(); i++)
	{
		std::vector<SAnimationKey> keys;
		keys.resize(asset.animationData[i].keys.size());
		for (usize j = 0; j < asset.animationData[i].keys.size(); j++)
		{
			keys[j].localBoneIndex = asset.animationData[i].keys[j].localBoneIndex;
			keys[j].positionKeys.resize(asset.animationData[i].keys[j].positionKeys.size());
			for (usize k = 0; k < asset.animationData[i].keys[j].positionKeys.size(); k++)
			{
				keys[j].positionKeys[k].position = cVector3(asset.animationData[i].keys[j].positionKeys[k].position);
				keys[j].positionKeys[k].time = asset.animationData[i].keys[j].positionKeys[k].time;
			}
			keys[j].rotationKeys.resize(asset.animationData[i].keys[j].rotationKeys.size());
			for (usize k = 0; k < asset.animationData[i].keys[j].rotationKeys.size(); k++)
			{
				keys[j].rotationKeys[k].rotation = cQuaternion(asset.animationData[i].keys[j].rotationKeys[k].rotation);
				keys[j].rotationKeys[k].time = asset.animationData[i].keys[j].rotationKeys[k].time;
			}
			keys[j].scaleKeys.resize(asset.animationData[i].keys[j].scaleKeys.size());
			for (usize k = 0; k < asset.animationData[i].keys[j].scaleKeys.size(); k++)
			{
				keys[j].scaleKeys[k].scale = cVector3(asset.animationData[i].keys[j].scaleKeys[k].scale);
				keys[j].scaleKeys[k].time = asset.animationData[i].keys[j].scaleKeys[k].time;
			}
		}

		m3dd.animations.push_back(_context->GetSubsystem<XAnimationSubsystem>()->CreateAnimation(
			asset.animationData[i].name,
			asset.animationData[i].duration,
			asset.animationData[i].ticksPerSecond,
			m3dd.skeleton,
			keys
		));
	}

	return model;
}

void triton::XModel3DSubsystem::DestroyModel(const HModel3D& model)
{
	SModel3DData& modelData = Get(model);
	_context->GetBackend<IModel3DBackend>()->DestroyModel(modelData);
	Destroy(model);
}