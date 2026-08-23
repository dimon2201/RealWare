layout(location = 0) out vec4 outPixelColor;

struct InputMaterial
{
	vec2 texcoord;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};

in InputMaterial vsOutputMaterial;
in vec3 pixelPositionWorldSpace;
in mat3 tangentToWorldMatrix;

layout(binding = 0) uniform sampler2D inDiffuseTexture;
layout(binding = 1) uniform sampler2D inNormalTexture;
layout(binding = 2) uniform sampler2D inRoughnessTexture;
layout(binding = 3) uniform sampler2D inMetallicTexture;
uniform uint time;
uniform vec4 cameraPositionWorldSpace;

void main()
{
	const vec4 diffuseColor = 
		vsOutputMaterial.diffuseColor *
		texture(inDiffuseTexture, vsOutputMaterial.texcoord);
	vec3 normal = texture(inNormalTexture, vsOutputMaterial.texcoord).xyz;
	const float roughness = texture(inRoughnessTexture, vsOutputMaterial.texcoord).x;
	const float metallic = texture(inMetallicTexture, vsOutputMaterial.texcoord).x;

	normal = normal * 2.0 - 1.0;
	normal = normalize(tangentToWorldMatrix * normal);

	const vec3 lightPositionWorldSpace = vec3(0.0f, 1.0f, 1.0f);
	const vec3 lightDirectionWorldSpace = normalize(lightPositionWorldSpace - pixelPositionWorldSpace);
	const vec3 viewDirectionWorldSpace = normalize(cameraPositionWorldSpace.xyz - pixelPositionWorldSpace);

	const vec3 pbrBRDF = PBR_BRDF(
		diffuseColor.xyz,
		roughness,
		metallic,
		normal,
		viewDirectionWorldSpace,
		lightDirectionWorldSpace
	);

	const float NdotL = max(dot(normal, lightDirectionWorldSpace), 0.0);
	const vec3 finalColor = pbrBRDF * NdotL;

	outPixelColor = vec4(finalColor, 1.0f);
}