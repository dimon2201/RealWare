#version 460

layout(location = 0) out vec4 outColor;

in vec2 texcoord;

layout(set = 0, binding = 0) uniform sampler2D presentTexture;

void main()
{
    outColor = texture(presentTexture, inTexcoord, 0);
}