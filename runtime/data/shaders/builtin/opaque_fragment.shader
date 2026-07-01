layout(location = 0) out vec4 FragColor;

in vec3 DiffuseTexcoordAtlas;
in vec3 NormalTexcoordAtlas;
in vec2 TexcoordOrig;
flat in vec4 DiffuseColor;
flat in mat3 TBNMatrix;
flat in mat3 TangentToWorld;

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
	vec4 textureColor = texture(TextureAtlas, DiffuseTexcoordAtlas);
	vec4 fragColor = vec4(0.0);
	
	Fragment_Passthrough(textureColor, DiffuseColor, fragColor);
	Fragment_Func(TexcoordOrig, textureColor, DiffuseColor, fragColor);

	// Normal mapping test
	const vec3 lightDir = vec3(0.0f, 0.0f, 1.0f);//normalize(vec3(0.0f, 0.0f, 5.0f) - vec3(0.0f));
	vec3 normal = texture(TextureAtlas, NormalTexcoordAtlas).xyz;
	normal = normal * 2.0 - 1.0;
	normal = normalize(TangentToWorld * normal);
	float NdotL = max(dot(normal, lightDir), 0.0);

	FragColor = fragColor * NdotL;
}