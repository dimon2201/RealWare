#version 430

layout(location = 0) in vec3 InPositionLocal;
layout(location = 1) in vec2 InTexcoord;
layout(location = 2) in vec3 InNormal;

struct Instance
{
	float Use2D;
	int MaterialIndex;
	uint _pad[2];
	mat4 World;
};

struct Material
{
	int BufferIndex;
	float DiffuseTextureLayerInfo;
	float MetallicTextureLayerInfo;
	float RoughnessTextureLayerInfo;
	float UserData[4];
	vec4 DiffuseTextureInfo;
	vec4 DiffuseColor;
	vec4 HighlightColor;
};

struct Light
{
	vec4 Position;
	vec4 Color;
	vec4 DirectionAndScale;
};

layout(std430, binding = 0) buffer InstanceBuffer { Instance instances[1024]; };
layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1024]; };
layout(std430, binding = 2) buffer LightBuffer { uvec4 lightCount; Light lights[1024]; };

out vec3 Texcoord;
flat out vec4 Color;
flat out uint LightCount;

uniform mat4 ViewProjection;

void main()
{
	Instance instance = instances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	Texcoord = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.DiffuseTextureLayerInfo);
	Texcoord.xy *= vec2(material.DiffuseTextureInfo.zw);
	Texcoord.xy += material.DiffuseTextureInfo.xy;
	Color = material.DiffuseColor * material.HighlightColor;
	LightCount = lightCount.x;

	if (instance.Use2D == 0) {
		gl_Position = ViewProjection * instance.World * vec4(InPositionLocal, 1.0);
	} else {
		gl_Position = instance.World * vec4(InPositionLocal, 1.0);
	}
}