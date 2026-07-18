// TRF.hpp

#pragma once

#include <optional>
#include <filesystem>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include "types.hpp"
#include "vertex.hpp"

namespace triton
{
	namespace resource_file
	{
		enum class EResourceFormat
		{
			Model3D
		};

		enum class ETextureFormat
		{
			PNG,
			DDS
		};

		enum class ETextureDataFormat
		{
			R8,
			RGB8,
			RGBA8,
			RGBA8_SRGB
		};

		struct SModel3DMaterialData;
		struct SModel3DBoneData;
		struct SModel3DAnimationData;
		struct SModel3DData
		{
			SVertex* vertexData = nullptr;
			types::u32* indexData = nullptr;
			types::usize vertexCount = 0;
			types::usize indexCount = 0;
			std::vector<SModel3DMaterialData> materialData;
			std::vector<SModel3DBoneData> boneData;
			std::vector<SModel3DAnimationData> animationData;
		};

		struct STextureData
		{
			ETextureFormat format;
			ETextureDataFormat dataFormat;
			types::usize width = 0;
			types::usize height = 0;
			types::usize channelCount = 0;
			types::u8* pixelByteData = nullptr;
		};

		struct SModel3DMaterialData
		{
			std::string diffuseTexturePath = "";
			std::string normalTexturePath = "";
			std::string roughnessTexturePath = "";
			std::string metallicTexturePath = "";
			types::boolean bIsDiffuseEmbedded = types::K_FALSE;
			types::boolean bIsNormalEmbedded = types::K_FALSE;
			types::boolean bIsRoughnessEmbedded = types::K_FALSE;
			types::boolean bIsMetallicEmbedded = types::K_FALSE;
			STextureData diffuseTexture;
			STextureData normalTexture;
			STextureData roughnessTexture;
			STextureData metallicTexture;
		};

		struct SModel3DVertexWeightData final
		{
			types::usize boneIndex = 0;
			types::f32 weight = 0.0f;
		};

		struct SModel3DBoneData
		{
			static constexpr types::s32 kNoParent = -1;
			static constexpr types::usize kMaxChildCount = 16;

			std::string name = {};
			glm::mat4 localMatrix = glm::mat4(1.0f);
			glm::mat4 modelMatrix = glm::mat4(1.0f);
			types::s32 parentLocalBoneIndex = kNoParent;
			std::vector<types::usize> childLocalBoneIndices = {};
		};

		struct SModel3DBonePositionKeyData
		{
			types::f32 time = 0.0f;
			glm::vec3 position = glm::vec3(0.0f);
		};

		struct SModel3DBoneRotationKeyData
		{
			types::f32 time = 0.0f;
			glm::quat rotation = glm::quat();
		};

		struct SModel3DBoneScaleKeyData
		{
			types::f32 time = 0.0f;
			glm::vec3 scale = glm::vec3(0.0f);
		};

		struct SModel3DAnimationKeyData
		{
			types::usize localBoneIndex = 0;
			std::vector<SModel3DBonePositionKeyData> positionKeys = {};
			std::vector<SModel3DBoneRotationKeyData> rotationKeys = {};
			std::vector<SModel3DBoneScaleKeyData> scaleKeys = {};
		};

		struct SModel3DAnimationData
		{
			std::string name = "";
			types::f32 duration = 0.0f;
			types::f32 ticksPerSecond = 0.0f;
			std::vector<SModel3DAnimationKeyData> keys = {};
		};

		template <EResourceFormat TResourceFormat>
		class CResourceFile
		{
			types::usize _dataByteSize = 0;
			types::u8* _data = nullptr;
			std::string _dataFolderPath = "";
			types::usize _parsedDataByteSize = 0;
			types::u8* _parsedData = nullptr;

		public:
			CResourceFile() = delete;
			explicit CResourceFile(const std::filesystem::path& filePath);
			~CResourceFile();

			void Parse();

		private:
			std::optional<SModel3DData> ParseModel3D();

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

			types::boolean CResourceFile::ParseDDS(
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
		};
	}
}