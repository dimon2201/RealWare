layout(location = 0) out vec4 FragColor;

in vec3 DiffuseTexcoordAtlas;
in vec3 NormalTexcoordAtlas;
in vec3 RoughnessTexcoordAtlas;
in vec3 MetallicTexcoordAtlas;
in vec2 TexcoordOrig;
in vec3 Normal;
in vec3 FragPosWorldSpace;
flat in vec4 DiffuseColor;
flat in mat3 TBNMatrix;
in mat3 TangentToWorld;
flat in uint OutTripleSixty;

struct TextureAtlasTexture
{
	vec4 TextureInfo;
	float TextureLayerInfo;
};
layout(std430, binding = 3) buffer TextureAtlasTexturesBuffer { TextureAtlasTexture textureAtlasTextures[1024]; };

layout(binding = 0) uniform sampler2DArray TextureAtlasRGBA8SRGB;
layout(binding = 1) uniform sampler2DArray TextureAtlasRGBA8;
layout(binding = 2) uniform sampler2DArray TextureAtlasR8;

void Fragment_Passthrough(in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor)
{
	_fragColor = _textureColor * _materialDiffuseColor;
}

void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor){}

uniform uint UniformTime;
uniform vec4 CameraPosWorldSpace;

void main()
{
	vec4 textureColor = texture(TextureAtlasRGBA8SRGB, DiffuseTexcoordAtlas);
	float roughness = texture(TextureAtlasR8, RoughnessTexcoordAtlas).x;
	float metallic = texture(TextureAtlasR8, MetallicTexcoordAtlas).x;
	vec4 fragColor = vec4(0.0);
	
	//Fragment_Passthrough(textureColor, DiffuseColor, fragColor);
	//Fragment_Func(TexcoordOrig, textureColor, DiffuseColor, fragColor);

	// Normal mapping test
	const vec3 lightPos = vec3(0.0f, 1.0f, 1.0f); //5.0f * vec3(cos(float(UniformTime * 0.0005f)), 0.0f, sin(float(UniformTime * 0.0005f))); //normalize(vec3(0.0f, 0.0f, 5.0f) - vec3(0.0f));
	const vec3 lightDir = normalize(lightPos - FragPosWorldSpace);
	vec3 normal = texture(TextureAtlasRGBA8, NormalTexcoordAtlas).xyz;
	normal = normal * 2.0 - 1.0;
	normal = normalize(TangentToWorld * normal);
	float NdotL = max(dot(normal, lightDir), 0.65);

	vec3 viewDir = normalize(CameraPosWorldSpace.xyz - FragPosWorldSpace);
	vec3 pbr = PBR(
		textureColor.xyz,
		1.0f,
		roughness,
		metallic,
		normal,
		viewDir,
		lightDir
	);

	FragColor = vec4(pow(pbr.xyz, vec3(1.0 / 2.2)), 1.0f);
}