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

layout(binding = 0) uniform sampler2DArray TextureAtlas;

void Fragment_Passthrough(in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor)
{
	_fragColor = _textureColor * _materialDiffuseColor;
}

void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}

void main()
{
	vec4 textureColor = texture(TextureAtlas, TexcoordAtlas);
	vec4 fragColor = vec4(0.0);
	
	Fragment_Passthrough(textureColor, DiffuseColor, fragColor);
	Fragment_Func(TexcoordOrig, textureColor, DiffuseColor, fragColor);
	FragColor = fragColor;
}