#version 430

out vec3 Texcoord;
flat out float UserData;
flat out vec4 DiffuseColor;

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
};

layout(std430, binding = 0) buffer InstanceBuffer { Instance instances[1024]; };
layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1024]; };

void main()
{
	vec3 PositionLocal = vec3(0.0);
	if (gl_VertexID == 0) { PositionLocal = vec3(0.0, 0.0, 0.0); }
	if (gl_VertexID == 1) { PositionLocal = vec3(0.0, 1.0, 0.0); }
	if (gl_VertexID == 2) { PositionLocal = vec3(1.0, 0.0, 0.0); }
	if (gl_VertexID == 3) { PositionLocal = vec3(1.0, 1.0, 0.0); }

	Instance instance = instances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	Texcoord = vec3(PositionLocal.x, 1.0 - PositionLocal.y, material.DiffuseTextureLayerInfo);
	Texcoord.xy *= material.DiffuseTextureInfo.zw;
	Texcoord.xy += material.DiffuseTextureInfo.xy;
	DiffuseColor = material.DiffuseColor;

	gl_Position = instance.World * vec4(PositionLocal, 1.0);
}