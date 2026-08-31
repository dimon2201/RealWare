#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 texcoord;

layout(set = 1, binding = 0) uniform sampler2D DiffuseTexture;

void main()
{
    outColor = texture(DiffuseTexture, texcoord, 0);
}