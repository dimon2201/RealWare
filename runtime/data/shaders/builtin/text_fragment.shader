layout(location = 0) out vec4 FragColor;

in vec3 TexcoordAtlas;
in vec2 TexcoordOrig;
flat in vec4 DiffuseColor;
flat in vec4 GlyphInfo;
flat in vec4 GlyphAtlasInfos;

struct TextureAtlasTexture
{
	vec4 TextureInfo;
	float TextureLayerInfo;
};
layout(std430, binding = 3) buffer TextureAtlasTexturesBuffer { TextureAtlasTexture textureAtlasTextures[1024]; };

uniform sampler2D FontAtlas;

void main()
{
	vec4 pixel = texture(FontAtlas, TexcoordAtlas.xy);
	if (pixel.x < 0.01) { discard; };
	FragColor = vec4(pixel.xxx, 1.0) * DiffuseColor;
}