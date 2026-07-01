layout(location = 0) in vec3 InPositionLocal;
layout(location = 1) in vec2 InTexcoord;
layout(location = 2) in vec3 InNormal;
layout(location = 3) in vec4 InTangent;

out vec3 DiffuseTexcoordAtlas;
out vec3 NormalTexcoordAtlas;
out vec2 TexcoordOrig;
flat out vec4 DiffuseColor;
flat out mat3 TBNMatrix;
flat out mat3 TangentToWorld;

uniform mat4 ViewProjection;
uniform uint InstanceBatchType;
uniform uint InstanceOffset;

struct Instance
{
	float Use2D;
	int MaterialIndex;
	mat4 World;
};

struct Texture
{
	uint AtlasLayer;
	vec2 AtlasNormOffset;
	vec2 AtlasNormSize;
};

struct Material
{
	Texture Diffuse;
	Texture Normal;
	vec4 DiffuseColor;
};

layout(std430, binding = 0) buffer StaticInstanceBuffer { Instance staticInstances[1024]; };
layout(std430, binding = 1) buffer DynamicInstanceBuffer { Instance dynamicInstances[1024]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[1024]; };

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
	Instance instance;
	if (InstanceBatchType == 0)
		instance = staticInstances[gl_InstanceID];
	else if (InstanceBatchType == 1)
		instance = dynamicInstances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	DiffuseTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Diffuse.AtlasLayer);
	DiffuseTexcoordAtlas.xy *= vec2(material.Diffuse.AtlasNormSize);
	DiffuseTexcoordAtlas.xy += material.Diffuse.AtlasNormOffset;
	NormalTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Normal.AtlasLayer);
	NormalTexcoordAtlas.xy *= vec2(material.Normal.AtlasNormSize);
	NormalTexcoordAtlas.xy += material.Normal.AtlasNormOffset;
	TexcoordOrig = InTexcoord;
	DiffuseColor = material.DiffuseColor;

	vec4 normalWS = instance.World * vec4(InNormal, 0.0f);
	vec4 tangentWS = instance.World * vec4(InTangent.xyz, 0.0f);
	vec3 bitangentLS = normalize(cross(InNormal, InTangent.xyz)) * InTangent.w;
	vec4 bitangentWS = instance.World * vec4(bitangentLS, 0.0f);
	TangentToWorld = mat3(tangentWS.xyz, bitangentWS.xyz, normalWS.xyz);

	Vertex_Passthrough(InPositionLocal, instance, 0, gl_Position);
	Vertex_Func(InPositionLocal, vec2(DiffuseTexcoordAtlas.xy), InNormal, gl_InstanceID, instance, material, 0, gl_Position);
}