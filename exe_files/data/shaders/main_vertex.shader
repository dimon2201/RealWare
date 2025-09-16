#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
layout(location = 0) in vec3 InPositionLocal;
layout(location = 1) in vec2 InTexcoord;
layout(location = 2) in vec3 InNormal;
#endif

out vec3 Texcoord;
flat out vec4 DiffuseColor;
#if defined(RENDER_PATH_TEXT)
flat out vec4 GlyphInfo;
flat out vec4 GlyphAtlasInfo;
#endif

#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
uniform mat4 ViewProjection;
#endif

#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
struct Instance
{
	float Use2D;
	int MaterialIndex;
	uint _pad[2];
	mat4 World;
};
#endif
#if defined(RENDER_PATH_TEXT)
struct TextInstance
{
	vec4 Info;
	vec4 AtlasInfo;
};
#endif

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

#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
layout(std430, binding = 0) buffer InstanceBuffer { Instance instances[1024]; };
#endif
#if defined(RENDER_PATH_TEXT)
layout(std430, binding = 0) buffer TextInstanceBuffer { TextInstance textInstances[1024]; };
#endif
layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1024]; };

#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
void Vertex_Transform(in vec3 _positionLocal, in Instance _instance, in float _use2D, out vec4 _glPosition)
{
	if (_use2D == 0) {
		_glPosition = ViewProjection * _instance.World * vec4(_positionLocal, 1.0);
	} else {
		_glPosition = _instance.World * vec4(_positionLocal, 1.0);
	}
}

void Vertex_Passthrough(in vec3 _positionLocal, in Instance _instance, in float _use2D, out vec4 _glPosition)
{
	Vertex_Transform(_positionLocal, _instance, _use2D, _glPosition);
}

void Vertex_Func(in vec3 _positionLocal, in vec3 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}
#endif

void main()
{
	#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
	Instance instance = instances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	Texcoord = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.DiffuseTextureLayerInfo);
	Texcoord.xy *= vec2(material.DiffuseTextureInfo.zw);
	Texcoord.xy += material.DiffuseTextureInfo.xy;
	DiffuseColor = material.DiffuseColor;

	Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);
	Vertex_Func(InPositionLocal, Texcoord, InNormal, gl_InstanceID, instance, material, instance.Use2D, gl_Position);
	#endif
	
	#if defined(RENDER_PATH_QUAD) || defined(RENDER_PATH_TRANSPARENT_COMPOSITE)
	vec3 PositionLocal = vec3(0.0);
	if (gl_VertexID == 0) { PositionLocal = vec3(-1.0, -1.0, 0.0); Texcoord = vec3(0.0, 0.0, 0.0); }
	if (gl_VertexID == 1) { PositionLocal = vec3(-1.0, 1.0, 0.0); Texcoord = vec3(0.0, 1.0, 0.0); }
	if (gl_VertexID == 2) { PositionLocal = vec3(1.0, -1.0, 0.0); Texcoord = vec3(1.0, 0.0, 0.0); }
	if (gl_VertexID == 3) { PositionLocal = vec3(1.0, 1.0, 0.0); Texcoord = vec3(1.0, 1.0, 0.0); }
	gl_Position = vec4(PositionLocal, 1.0);
	#endif
	
	#if defined(RENDER_PATH_TEXT)
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
	Texcoord = vec3(PositionLocal.x, 1.0 - PositionLocal.y, 0.0);
	Texcoord *= GlyphAtlasInfo.zww;
	Texcoord += GlyphAtlasInfo.xyy;
	PositionLocal.xy *= GlyphInfo.zw;
	PositionLocal.xy += GlyphInfo.xy;

	gl_Position = vec4(PositionLocal, 1.0);
	#endif
}