layout(location = 0) out vec4 outPixelColor;

struct InputMaterial
{
	vec3 diffuseAtlasTexcoord;
	vec3 normalAtlasTexcoord;
	vec3 roughnessAtlasTexcoord;
	vec3 metallicAtlasTexcoord;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};

in InputMaterial vsOutputMaterial;
in vec3 pixelPositionWorldSpace;
in mat3 tangentToWorldMatrix;

layout(binding = 0) uniform sampler2DArray textureAtlasRGBA8SRGB;
layout(binding = 1) uniform sampler2DArray textureAtlasRGBA8;
layout(binding = 2) uniform sampler2DArray textureAtlasR8;
uniform uint time;
uniform vec4 cameraPositionWorldSpace;

void main()
{
	const vec4 diffuseColor = 
		vsOutputMaterial.diffuseColor *
		texture(textureAtlasRGBA8SRGB, vsOutputMaterial.diffuseAtlasTexcoord);
	vec3 normal = texture(textureAtlasRGBA8, vsOutputMaterial.normalAtlasTexcoord).xyz;

	normal = normal * 2.0 - 1.0;
	normal = normalize(tangentToWorldMatrix * normal);

	const vec3 lightPositionWorldSpace = vec3(0.0f, 100.0f, 0.0f);
	const vec3 lightDirectionWorldSpace = normalize(lightPositionWorldSpace - pixelPositionWorldSpace);
	const vec3 viewDirectionWorldSpace = normalize(cameraPositionWorldSpace.xyz - pixelPositionWorldSpace);

	const vec3 phongBRDF = BlinnPhong_BRDF(
		diffuseColor.xyz,
		vsOutputMaterial.specularColor.xyz,
		vsOutputMaterial.shininess,
		normal,
		viewDirectionWorldSpace,
		lightDirectionWorldSpace
	);

	const float NdotL = max(dot(normal, lightDirectionWorldSpace), 0.0);
	const vec3 finalColor = phongBRDF * NdotL;

	outPixelColor = vec4(finalColor, 1.0f);
}