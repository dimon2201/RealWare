#version 460

#include "brdf.shader"

struct OutputMaterial
{
	vec2 texcoord;
	vec4 diffuseColor;
	vec4 specularColor;
	float shininess;
};

layout(location = 0) out vec4 outColor;

layout(location = 0) in OutputMaterial vsOutputMaterial;
layout(location = 4) in vec3 pixelPositionWorldSpace;
layout(location = 5) in mat3 tangentToWorldMatrix;
layout(location = 8) in vec3 cameraPositionWorldSpace;
layout(location = 9) in float time;

layout(set = 1, binding = 0) uniform sampler2D DiffuseTexture;
layout(set = 1, binding = 1) uniform sampler2D NormalTexture;
layout(set = 1, binding = 2) uniform sampler2D RoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D MetallicTexture;

void main()
{
    const vec4 diffuseColor = 
		vsOutputMaterial.diffuseColor *
		texture(DiffuseTexture, vsOutputMaterial.texcoord);

	vec3 normal = texture(NormalTexture, vsOutputMaterial.texcoord).xyz;
	const float roughness = texture(RoughnessTexture, vsOutputMaterial.texcoord).x;
	const float metallic = texture(MetallicTexture, vsOutputMaterial.texcoord).x;

	normal = normal * 2.0 - 1.0;
	normal = normalize(tangentToWorldMatrix * normal);

	const vec3 lightPositionWorldSpace = vec3(sin(time * 0.025f) * 100.0f, 100.0f, cos(time * 0.025f) * 100.0f);
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

	outColor = vec4(finalColor, 1.0f);
}