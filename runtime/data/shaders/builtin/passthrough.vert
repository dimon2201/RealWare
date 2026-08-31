#version 460

struct OutputMaterial
{
	vec2 texcoord;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};

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

layout(location = 0) in vec3 inPositionLocalSpace;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec4 inTangent;

layout(location = 0) out OutputMaterial vsOutputMaterial;
layout(location = 4) out vec3 pixelPositionWorldSpace;
layout(location = 5) out mat3 tangentToWorldMatrix;
layout(location = 8) out vec3 cameraPositionWorldSpace;
layout(location = 9) out float time;

layout(push_constant, std430) uniform PushConstants
{
    vec3 cameraPositionWorldSpace;
    uint instanceMotionType;
    mat4 cameraViewProjectionMatrix;
    float time;
} pushConstants;

layout(set = 0, binding = 0, std430) readonly buffer StaticInstanceBuffer
{
    Instance instances[];
} staticInstanceBuffer;

layout(set = 0, binding = 1, std430) readonly buffer DynamicInstanceBuffer
{
    Instance instances[];
} dynamicInstanceBuffer;

layout(set = 0, binding = 2, std430) readonly buffer MaterialBuffer
{
    Material materials[];
} materialBuffer;

void main()
{
	Instance instance;

	if (pushConstants.instanceMotionType == 1)
		instance = staticInstanceBuffer.instances[gl_InstanceIndex];
	else if (pushConstants.instanceMotionType == 2)
		instance = dynamicInstanceBuffer.instances[gl_InstanceIndex];

	Material material = materialBuffer.materials[instance.materialIndex];

    vsOutputMaterial.texcoord = vec2(inTexcoord.x, 1.0 - inTexcoord.y);
	vsOutputMaterial.diffuseColor = material.diffuseColor;
	vsOutputMaterial.specularColor = material.specularColor;
	vsOutputMaterial.shininess = material.shininess;

	pixelPositionWorldSpace = vec3(instance.worldMatrix * vec4(inPositionLocalSpace, 1.0));

	mat3 normalMatrix = transpose(inverse(mat3(instance.worldMatrix)));
	vec3 N = normalize(normalMatrix * inNormal);
	vec3 T = normalize(normalMatrix * inTangent.xyz);
	vec3 B = normalize(cross(N, T)) * inTangent.w;
	tangentToWorldMatrix = mat3(T, B, N);

	cameraPositionWorldSpace = pushConstants.cameraPositionWorldSpace;

	time = pushConstants.time;

    gl_Position =
        pushConstants.cameraViewProjectionMatrix *
        vec4(pixelPositionWorldSpace, 1.0);
}