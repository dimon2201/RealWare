// model3d.cpp

#include "model3d.hpp"
#include "context.hpp"
#include "tasset.hpp"
#include "material_pool.hpp"
#include "skeleton_pool.hpp"
#include "animation_pool.hpp"
#include "object_allocator.hpp"
#include "texture.hpp"
#include "image_decoder.hpp"
#include "graphics_texture_usage_enum.hpp"

using namespace types;

triton::XModel3D::XModel3D(
    cContext* context,
    types::s32 poolIndex,
	EVertexBufferFormat vertexDataFormat,
    EModel3DFileType fileType,
    const std::filesystem::path& filePath
) : iObject(context, poolIndex), _vertexDataFormat(vertexDataFormat), _fileType(fileType)
{
    if (_fileType == EModel3DFileType::Raw)
    {
		std::filesystem::path modelFolderPath = filePath.parent_path();
		std::filesystem::path modelLocalPath = filePath.filename();

		auto result = _context->GetBackend<IModel3DBackend>()->CreateModel(
			modelFolderPath.generic_string(),
			modelLocalPath.generic_string(),
			vertexDataFormat
		);
		if (!result)
			return;

        _data = *result;
    }
    else if (_fileType == EModel3DFileType::Asset)
    {
		std::filesystem::path assetFolderPath = filePath.parent_path();

		_asset = CObjectAllocator::Create<asset::CModel3DAsset>(64);
		_asset->LoadAssetFile(filePath);

		// TODO: rewrite TAsset tools to rigid/skinned models support
		//_data.rigidVertexData = (const SRigidVertexGPULayout*)_asset->rigidVertexData;
		//_data.skinnedVertexData = (const SSkinnedVertexGPULayout*)_asset->skinnedVertexData;
		_data.vertexCount = _asset->vertexCount;
		_data.indexData = _asset->indexData;
		_data.indexCount = _asset->indexCount;

		for (usize i = 0; i < _asset->materialData.size(); i++)
		{
			asset::SModel3DMaterialData& assetMaterialData = _asset->materialData[i];

			auto ProcessTexture = [this](
				const std::filesystem::path& assetFolderPath,
				const std::filesystem::path& assetTexturePath,
				const asset::STextureData& assetTextureData,
				ETextureFormat textureFormat,
				XTexture::THandle& outHandle
				) {
					std::filesystem::path texturePath = assetTexturePath;
					if (!assetTexturePath.is_absolute())
						texturePath = assetFolderPath / assetTexturePath;
					if (std::filesystem::exists(texturePath))
						outHandle = *_context->GetPool<PTexturePool>()->Create(
							types::K_FALSE,
							textureFormat,
							(dword)ETextureUsageBit::Sampled,
							ETextureDimension::Texture2D,
							0,
							_context->GetBackend<IImageDecoder>()->TextureFormatToImageFormat(textureFormat),
							texturePath.generic_string()
						);
					else
						outHandle = *_context->GetPool<PTexturePool>()->Create(
							types::K_FALSE,
							textureFormat,
							(dword)ETextureUsageBit::Sampled,
							ETextureDimension::Texture2D,
							cVector3(assetTextureData.width, assetTextureData.height, 0),
							assetTextureData.pixelByteData,
							0
						);
				};

			XTexture::THandle diffuseTexture;
			XTexture::THandle normalTexture;
			XTexture::THandle roughnessTexture;
			XTexture::THandle metallicTexture;

			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.diffuseTexturePath),
				assetMaterialData.diffuseTexture,
				ETextureFormat::RGBA8_SRGB_Mips,
				diffuseTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.normalTexturePath),
				assetMaterialData.normalTexture,
				ETextureFormat::RGBA8,
				normalTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.roughnessTexturePath),
				assetMaterialData.roughnessTexture,
				ETextureFormat::R8,
				roughnessTexture
			);
			ProcessTexture(
				assetFolderPath,
				std::filesystem::path(assetMaterialData.metallicTexturePath),
				assetMaterialData.metallicTexture,
				ETextureFormat::R8,
				metallicTexture
			);

			XMaterial::THandle material = *_context->GetPool<CMaterialPool>()->Create(
				diffuseTexture,
				normalTexture,
				roughnessTexture,
				metallicTexture,
				cVector4(1.0f),
				cVector4(1.0f),
				0.0f
			);

			_data.materials.push_back(material);
		}

		std::vector<SSkeletonBone> bones;
		bones.resize(_asset->boneData.size());
		for (usize i = 0; i < _asset->boneData.size(); i++)
		{
			bones[i].name = _asset->boneData[i].name;
			bones[i].localMatrix = cMatrix4(_asset->boneData[i].localMatrix);
			bones[i].modelMatrix = cMatrix4(_asset->boneData[i].modelMatrix);
			bones[i].localParentBoneIndex = _asset->boneData[i].parentLocalBoneIndex;
			bones[i].localChildBoneIndices = _asset->boneData[i].childLocalBoneIndices;
		}

		if (bones.size() > 0)
		{
			_data.skeleton = *_context->GetPool<CSkeletonPool>()->Create(
				bones,
				cMatrix4(_asset->rootBoneAccumulatedTransform)
			);
		}

		for (usize i = 0; i < _asset->animationData.size(); i++)
		{
			std::vector<SAnimationKey> keys;
			keys.resize(_asset->animationData[i].keys.size());
			for (usize j = 0; j < _asset->animationData[i].keys.size(); j++)
			{
				keys[j].localBoneIndex = _asset->animationData[i].keys[j].localBoneIndex;
				keys[j].positionKeys.resize(_asset->animationData[i].keys[j].positionKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].positionKeys.size(); k++)
				{
					keys[j].positionKeys[k].position = cVector3(_asset->animationData[i].keys[j].positionKeys[k].position);
					keys[j].positionKeys[k].time = _asset->animationData[i].keys[j].positionKeys[k].time;
				}
				keys[j].rotationKeys.resize(_asset->animationData[i].keys[j].rotationKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].rotationKeys.size(); k++)
				{
					keys[j].rotationKeys[k].rotation = cQuaternion(_asset->animationData[i].keys[j].rotationKeys[k].rotation);
					keys[j].rotationKeys[k].time = _asset->animationData[i].keys[j].rotationKeys[k].time;
				}
				keys[j].scaleKeys.resize(_asset->animationData[i].keys[j].scaleKeys.size());
				for (usize k = 0; k < _asset->animationData[i].keys[j].scaleKeys.size(); k++)
				{
					keys[j].scaleKeys[k].scale = cVector3(_asset->animationData[i].keys[j].scaleKeys[k].scale);
					keys[j].scaleKeys[k].time = _asset->animationData[i].keys[j].scaleKeys[k].time;
				}
			}

			XAnimation::THandle animHandle = *_context->GetPool<CAnimationPool>()->Create(
				_asset->animationData[i].name,
				_asset->animationData[i].duration,
				_asset->animationData[i].ticksPerSecond,
				keys
			);

			_data.animations.push_back(animHandle);
		}
    }
}

triton::XModel3D::~XModel3D()
{
	if (_fileType == EModel3DFileType::Raw)
	{
		_context->GetBackend<IModel3DBackend>()->DestroyModel(_data);
	}
	else if (_fileType == EModel3DFileType::Asset)
	{
		_asset->Destroy();
		CObjectAllocator::Destroy<asset::CModel3DAsset>(_asset);
	}
}