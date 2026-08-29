#version 460

layout(location = 0) in vec3 inPositionLocalSpace;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 color;

layout(push_constant, std430) uniform PushConstants
{
    vec3 cameraWorldPosition;
    uint instanceMotionType;
    mat4 cameraViewProjectionMatrix;
    float time;
} pushConstants;

void main()
{
    const vec3 colors[] = {
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    };

    color = colors[gl_VertexIndex];

    gl_Position = pushConstants.cameraViewProjectionMatrix * vec4(inPositionLocalSpace, 1.0);
}