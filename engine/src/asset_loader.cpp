// asset_loader.cpp

#include "asset_loader.hpp"
#include "model3d_backend.hpp"
#include "model3d_pool.hpp"
#include "tasset.hpp"
#include "material_subsystem.hpp"
#include "skeleton_subsystem.hpp"
#include "animation_subsystem.hpp"

using namespace types;

triton::XAssetLoader::XAssetLoader(cContext* context) : iObject(context)
{
	_model3dPool = CObjectAllocator::Create<XModel3DPool>(64, _context, K_FALSE);
}

triton::XAssetLoader::~XAssetLoader()
{
	CObjectAllocator::Destroy<XModel3DPool>(_model3dPool);
}

std::optional<triton::SModel3DData::THandle> triton::XAssetLoader::LoadModel(const std::filesystem::path& path)
{
	const auto ext = path.extension();
	if (ext == "tasset")
	{
		std::filesystem::path assetFolderPath = path.parent_path();

		asset::CModel3DAsset asset;
		asset.LoadAssetFile(path);

		auto handleResult = _model3dPool->Create();
		if (!handleResult.has_value())
			return std::nullopt;
		auto handle = *handleResult;

		auto dataResult = _model3dPool->Get(handle);
		if (!dataResult.has_value())
			return std::nullopt;
		SModel3DData& data = *dataResult;

		data.vertexData = (const SSkinnedVertexGPULayout*)asset.vertexData;
		data.vertexCount = asset.vertexCount;
		data.indexData = asset.indexData;
		data.indexCount = asset.indexCount;

		for (usize i = 0; i < asset.materialData.size(); i++)
		{
			asset::SModel3DMaterialData& assetMaterialData = asset.materialData[i];

			auto ProcessTexture = [](
				const std::filesystem::path& assetFolderPath,
				const std::filesystem::path& assetTexturePath,
				const asset::STextureData& assetTextureData,
				cTexture::eFormat textureFormat,
				XTextureSubsystem* textureSubsystem,
				STextureData::THandle& outHandle
			) {
				std::filesystem::path texturePath = assetTexturePath;
				if (!assetTexturePath.is_absolute())
					texturePath = assetFolderPath / assetTexturePath;
				if (std::filesystem::exists(texturePath))
					outHandle = *textureSubsystem->Create(
						texturePath.generic_string(),
						textureFormat
					);
				else
					outHandle = *textureSubsystem->Create(
						assetTextureData.pixelByteData,
						assetTextureData.width * assetTextureData.height * assetTextureData.channelCount,
						assetTextureData.width,
						assetTextureData.height,
						assetTextureData.channelCount,
						ETextureFormat::Raw,
						textureFormat
					);
			};

			STextureData::THandle diffuseTexture;
			STextureData::THandle normalTexture;
			STextureData::THandle roughnessTexture;
			STextureData::THandle metallicTexture;
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

			SMaterialData::THandle material = *_context->GetSubsystem<XMaterialSubsystem>()->Create(
				cVector4(1.0f),
				diffuseTexture,
				normalTexture,
				roughnessTexture,
				metallicTexture
			);

			data.materials.push_back(material);
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
			data.skeleton = *_context->GetSubsystem<XSkeletonSubsystem>()->Create(
				bones,
				cMatrix4(asset.rootBoneAccumulatedTransform)
			);
		}
		else
		{
			data.skeleton.Invalidate();
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

			SAnimationData::THandle animHandle = *_context->GetSubsystem<XAnimationSubsystem>()->Create(
				asset.animationData[i].name,
				asset.animationData[i].duration,
				asset.animationData[i].ticksPerSecond,
				keys
			);

			data.animations.push_back(animHandle);
		}

		return handle;
	}
	else if (ext == "fbx")
	{
		std::filesystem::path modelFolderPath = path.parent_path();
		std::filesystem::path modelLocalPath = path.filename();

		auto modelResult = _context->GetBackend<IModel3DBackend>()->CreateModel(
			modelFolderPath.generic_string(),
			modelLocalPath.generic_string()
		);
		if (!modelResult)
			return std::nullopt;

		auto handleResult = _model3dPool->Create();
		if (!handleResult.has_value())
			return std::nullopt;
		auto handle = *handleResult;

		auto dataResult = _model3dPool->Get(handle);
		if (!dataResult.has_value())
			return std::nullopt;
		SModel3DData& data = *dataResult;
		data = *modelResult;

		return handle;
	}
}