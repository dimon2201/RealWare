layout(location = 0) in vec3 InPositionLocal;
layout(location = 1) in vec2 InTexcoord;
layout(location = 2) in vec3 InNormal;

out vec3 TexcoordAtlas;
out vec2 TexcoordOrig;
flat out vec4 DiffuseColor;

uniform mat4 ViewProjection;
uniform uint InstanceBatchType;
uniform uint InstanceOffset;

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

layout(std430, binding = 0) buffer StaticInstanceBuffer { Instance instances[1024]; };
layout(std430, binding = 1) buffer DynamicInstanceBuffer { Instance instances[1024]; };
layout(std430, binding = 1) buffer MaterialBuffer { Material materials[1024]; };

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

void Vertex_Func(in vec3 _positionLocal, in vec2 _texcoord, in vec3 _normal, in int _instanceID, in Instance _instance, in Material material, in float _use2D, out vec4 _glPosition){}

void main()
{
	Instance instance = instances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	TexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.DiffuseTextureLayerInfo);
	TexcoordAtlas.xy *= vec2(material.DiffuseTextureInfo.zw);
	TexcoordAtlas.xy += material.DiffuseTextureInfo.xy;
	TexcoordOrig = InTexcoord;
	DiffuseColor = material.DiffuseColor;

	Vertex_Passthrough(InPositionLocal, instance, instance.Use2D, gl_Position);
	Vertex_Func(InPositionLocal, TexcoordOrig, InNormal, gl_InstanceID, instance, material, instance.Use2D, gl_Position);
}