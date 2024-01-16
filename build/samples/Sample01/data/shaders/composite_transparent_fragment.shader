#version 430

layout(location = 0) out vec4 FragColor;

in vec2 Texcoord;

uniform sampler2D AccumulationTexture;
uniform sampler2D RevealageTexture;

void main()
{
	vec2 size = vec2(textureSize(AccumulationTexture, 0));

	vec4 accumulation = texelFetch(AccumulationTexture, ivec2(Texcoord * size), 0);
	float revealage = texelFetch(RevealageTexture, ivec2(Texcoord * size), 0).x;

	FragColor = vec4(accumulation.rgb / max(accumulation.a, 0.00001), 1.0 - revealage);
}