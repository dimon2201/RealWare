#version 430

out vec2 Texcoord;
flat out vec4 GlyphInfo;
flat out vec4 GlyphAtlasInfo;
flat out vec4 DiffuseColor;

struct TextInstance
{
	vec4 Info;
	vec4 AtlasInfo;
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

layout(std430, binding = 0) buffer TextInstanceBuffer { TextInstance textInstances[1024]; };
layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1]; };

void main()
{
	vec3 PositionLocal = vec3(0.0);
	if (gl_VertexID == 0) { PositionLocal = vec3(0.0, 0.0, 0.0); }
	if (gl_VertexID == 1) { PositionLocal = vec3(0.0, 1.0, 0.0); }
	if (gl_VertexID == 2) { PositionLocal = vec3(1.0, 0.0, 0.0); }
	if (gl_VertexID == 3) { PositionLocal = vec3(1.0, 1.0, 0.0); }

	TextInstance textInstance = textInstances[gl_InstanceID];
	Material material = materials[0];

	GlyphInfo = textInstance.Info;
	GlyphAtlasInfo = textInstance.AtlasInfo;
	DiffuseColor = material.DiffuseColor;
	Texcoord = vec2(PositionLocal.x, 1.0 - PositionLocal.y);
	Texcoord *= GlyphAtlasInfo.zw;
	Texcoord += GlyphAtlasInfo.xy;
	PositionLocal.xy *= GlyphInfo.zw;
	PositionLocal.xy += GlyphInfo.xy;

	gl_Position = vec4(PositionLocal, 1.0);
}