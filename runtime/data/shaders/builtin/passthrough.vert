#version 460

struct Instance
{
    float use2D;
	int materialIndex;
	int skinnedBoneBufferOffset;
	uint propertyBits;
	mat4 worldMatrix;
};

layout(location = 0) in vec3 inPositionLocalSpace;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec2 texcoord;

layout(push_constant, std430) uniform PushConstants
{
    vec3 cameraWorldPosition;
    uint instanceMotionType;
    mat4 cameraViewProjectionMatrix;
    float time;
} pushConstants;

layout(set = 0, binding = 0, std430) readonly buffer RigidInstanceBuffer
{
    Instance instances[];
} rigidInstanceBuffer;

void main()
{
    texcoord = inTexcoord;
    texcoord.y = 1.0 - texcoord.y;

    gl_Position =
        pushConstants.cameraViewProjectionMatrix *
        rigidInstanceBuffer.instances[0].worldMatrix *
        vec4(inPositionLocalSpace, 1.0);
}