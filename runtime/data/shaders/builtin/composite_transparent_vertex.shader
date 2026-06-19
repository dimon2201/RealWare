out vec3 TexcoordAtlas;
out vec2 TexcoordOrig;
flat out vec4 DiffuseColor;

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

layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1024]; };

void main()
{
	vec3 PositionLocal = vec3(0.0);
	if (gl_VertexID == 0) { PositionLocal = vec3(-1.0, -1.0, 0.0); TexcoordAtlas = vec3(0.0, 0.0, 0.0); }
	if (gl_VertexID == 1) { PositionLocal = vec3(-1.0, 1.0, 0.0); TexcoordAtlas = vec3(0.0, 1.0, 0.0); }
	if (gl_VertexID == 2) { PositionLocal = vec3(1.0, -1.0, 0.0); TexcoordAtlas = vec3(1.0, 0.0, 0.0); }
	if (gl_VertexID == 3) { PositionLocal = vec3(1.0, 1.0, 0.0); TexcoordAtlas = vec3(1.0, 1.0, 0.0); }
	gl_Position = vec4(PositionLocal, 1.0);
}