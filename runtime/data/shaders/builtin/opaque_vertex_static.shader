layout(location = 0) in vec3 inPositionLocal;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;
layout(location = 4) in int	inMaterialIndex;

struct Instance
{
	float use2D;
	int materialIndex;
	int skinnedBoneBufferOffset;
	uint propertyBits;
	mat4 worldMatrix;
};

struct Texture
{
	uint atlasLayer;
	vec2 atlasNormOffset;
	vec2 atlasNormSize;
};

struct Material
{
	Texture diffuseTexture;
	Texture normalTexture;
	Texture roughnessTexture;
	Texture metallicTexture;
	vec4 diffuseColor;
};

struct Skinning
{
	mat4 modelMatrix;
};

struct OutputMaterial
{
	vec4 diffuseColor;
	vec3 diffuseAtlasTexcoord;
	vec3 normalAtlasTexcoord;
	vec3 roughnessAtlasTexcoord;
	vec3 metallicAtlasTexcoord;
};

layout(std430, binding = 0) buffer StaticInstanceBuffer { Instance staticInstances[]; };
layout(std430, binding = 1) buffer DynamicInstanceBuffer { Instance dynamicInstances[]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[]; };
layout(std430, binding = 3) buffer SkinningBuffer { Skinning skinning[]; };

out OutputMaterial vsOutputMaterial;
out vec3 pixelPositionWorldSpace;
out mat3 tangentToWorldMatrix;

uniform mat4 viewProjectionMatrix;
uniform uint instanceBatchType;
uniform uint instanceOffset;

vec3 CalculateAtlasTexcoord(in Texture atlasTexture)
{
	vec3 atlasTexcoord = vec3(0.0f);
	atlasTexcoord = vec3(inTexcoord.x, 1.0 - inTexcoord.y, atlasTexture.atlasLayer);
	atlasTexcoord.xy *= vec2(atlasTexture.atlasNormSize);
	atlasTexcoord.xy += atlasTexture.atlasNormOffset;

	return atlasTexcoord;
}

void VertexTransform(in mat4 worldMatrix)
{
	gl_Position = viewProjectionMatrix * worldMatrix * vec4(inPositionLocal, 1.0);
}

void main()
{
	Instance instance;
	if (instanceBatchType == 1)
		instance = staticInstances[gl_InstanceID + instanceOffset];
	else if (instanceBatchType == 2)
		instance = dynamicInstances[gl_InstanceID + instanceOffset];
	
	Material material;
	material = materials[instance.materialIndex];
	
	vsOutputMaterial.diffuseColor = material.diffuseColor;
	vsOutputMaterial.diffuseAtlasTexcoord = CalculateAtlasTexcoord(material.diffuseTexture);
	vsOutputMaterial.normalAtlasTexcoord = CalculateAtlasTexcoord(material.normalTexture);
	vsOutputMaterial.roughnessAtlasTexcoord = CalculateAtlasTexcoord(material.roughnessTexture);
	vsOutputMaterial.metallicAtlasTexcoord = CalculateAtlasTexcoord(material.metallicTexture);

	pixelPositionWorldSpace = vec3(instance.worldMatrix * vec4(inPositionLocal, 1.0));

	mat3 normalMatrix = transpose(inverse(mat3(instance.worldMatrix)));
	vec3 N = normalize(normalMatrix * inNormal);
	vec3 T = normalize(normalMatrix * inTangent.xyz);
	vec3 B = normalize(cross(N, T)) * inTangent.w;
	tangentToWorldMatrix = mat3(T, B, N);

	VertexTransform(instance.worldMatrix);
}