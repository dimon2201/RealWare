layout(location = 0) out vec4 Accumulation;
layout(location = 1) out float Revealage;

in vec3 TexcoordAtlas;
in vec2 TexcoordOrig;
flat in vec4 DiffuseColor;

struct TextureAtlasTexture
{
	vec4 TextureInfo;
	float TextureLayerInfo;
};
layout(std430, binding = 3) buffer TextureAtlasTexturesBuffer { TextureAtlasTexture textureAtlasTextures[1024]; };

uniform sampler2DArray TextureAtlas;

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

	float weight = clamp(pow(min(1.0, fragColor.a * 10.0) + 0.01, 3.0) * 1e8 *
	   pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	Accumulation = vec4(fragColor.rgb * fragColor.a, fragColor.a) * weight;
	Revealage = fragColor.a;
}