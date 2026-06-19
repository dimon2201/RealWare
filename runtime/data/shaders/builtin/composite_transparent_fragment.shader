layout(location = 0) out vec4 FragColor;

in vec3 TexcoordAtlas;
in vec2 TexcoordOrig;
flat in vec4 DiffuseColor;

struct TextureAtlasTexture
{
	vec4 TextureInfo;
	float TextureLayerInfo;
};
layout(std430, binding = 3) buffer TextureAtlasTexturesBuffer { TextureAtlasTexture textureAtlasTextures[1024]; };

uniform sampler2D AccumulationTexture;
uniform sampler2D RevealageTexture;

void main()
{
	vec2 size = vec2(textureSize(AccumulationTexture, 0));

	vec4 accumulation = texelFetch(AccumulationTexture, ivec2(TexcoordAtlas.xy * size), 0);
	float revealage = texelFetch(RevealageTexture, ivec2(TexcoordAtlas.xy * size), 0).x;

	FragColor = vec4(accumulation.rgb / max(accumulation.a, 0.00001), 1.0 - revealage);
}