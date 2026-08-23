layout(location = 0) in vec3 inPositionLocal;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

struct Instance
{
	float use2D;
	int materialIndex;
	int skinnedBoneBufferOffset;
	uint propertyBits;
	mat4 worldMatrix;
};

struct Material
{
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};

struct Skinning
{
	mat4 modelMatrix;
};

struct OutputMaterial
{
	vec2 texcoord;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
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
	
	vsOutputMaterial.texcoord = vec2(inTexcoord.x, 1.0 - inTexcoord.y);
	vsOutputMaterial.diffuseColor = material.diffuseColor;
	vsOutputMaterial.specularColor = material.specularColor;
	vsOutputMaterial.shininess = material.shininess;

	pixelPositionWorldSpace = vec3(instance.worldMatrix * vec4(inPositionLocal, 1.0));

	mat3 normalMatrix = transpose(inverse(mat3(instance.worldMatrix)));
	vec3 N = normalize(normalMatrix * inNormal);
	vec3 T = normalize(normalMatrix * inTangent.xyz);
	vec3 B = normalize(cross(N, T)) * inTangent.w;
	tangentToWorldMatrix = mat3(T, B, N);

	VertexTransform(instance.worldMatrix);
}