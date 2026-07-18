// TRF.hpp

#pragma once

#include <optional>
#include <filesystem>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include "types.hpp"

namespace triton
{
	namespace resource_file
	{
		void Print(const std::string& message);

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

		struct SVertex
		{
			static constexpr types::usize kMaxBonesPerVertex = 4;

			glm::vec3 position = glm::vec3(0.0f);
			glm::vec2 texcoord = glm::vec2(0.0f);
			glm::vec3 normal = glm::vec3(0.0f);
			glm::vec4 tangent = glm::vec4(0.0f);
			types::s32 materialIndex = -1;
			types::u32 boneIndices[4] = {};
			types::f32 boneWeights[4] = {};
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
			std::string _dataLocalFilePath = "";
			types::usize _parsedDataByteSize = 0;
			types::u8* _parsedData = nullptr;

		public:
			CResourceFile() = delete;
			CResourceFile(const CResourceFile& other) = delete;
			CResourceFile& operator=(const CResourceFile& other) = delete;
			CResourceFile(CResourceFile&& other) = delete;
			CResourceFile& operator=(CResourceFile&& other) = delete;
			~CResourceFile();

			explicit CResourceFile(const std::filesystem::path& filePath);

			void Parse();
		};
	}
}