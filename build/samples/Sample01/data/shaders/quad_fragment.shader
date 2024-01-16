#version 430

layout(location = 0) out vec4 FragColor;

in vec2 Texcoord;

uniform sampler2D ColorTexture;

void main()
{
	vec2 size = vec2(textureSize(ColorTexture, 0));

	FragColor = texelFetch(ColorTexture, ivec2(Texcoord * size), 0);
}