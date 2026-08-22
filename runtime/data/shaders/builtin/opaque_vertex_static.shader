layout(location = 0) in vec3 InPositionLocal;
layout(location = 1) in vec2 InTexcoord;
layout(location = 2) in vec3 InNormal;
layout(location = 3) in vec4 InTangent;
layout(location = 4) in int InMaterialIndex;

out vec3 DiffuseTexcoordAtlas;
out vec3 NormalTexcoordAtlas;
out vec3 RoughnessTexcoordAtlas;
out vec3 MetallicTexcoordAtlas;
out vec2 TexcoordOrig;
out vec3 Normal;
out vec3 FragPosWorldSpace;
flat out vec4 DiffuseColor;
flat out mat3 TBNMatrix;
out mat3 TangentToWorld;

uniform mat4 ViewProjection;
uniform uint InstanceBatchType;
uniform uint InstanceOffset;

struct Instance
{
	float Use2D;
	int MaterialIndex;
	int SkinnedBoneBufferOffset;
	uint PropertyBits;
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
	Texture Roughness;
	Texture Metallic;
	vec4 DiffuseColor;
};

struct Skinning
{
	mat4 modelMatrix;
};

layout(std430, binding = 0) buffer StaticInstanceBuffer { Instance staticInstances[]; };
layout(std430, binding = 1) buffer DynamicInstanceBuffer { Instance dynamicInstances[]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[]; };
layout(std430, binding = 3) buffer SkinningBuffer { Skinning skinning[]; };

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
	if (InstanceBatchType == 1)
		instance = staticInstances[gl_InstanceID + InstanceOffset];
	else if (InstanceBatchType == 2)
		instance = dynamicInstances[gl_InstanceID + InstanceOffset];
	Material material;
	material = materials[instance.MaterialIndex];
	
	DiffuseTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Diffuse.AtlasLayer);
	DiffuseTexcoordAtlas.xy *= vec2(material.Diffuse.AtlasNormSize);
	DiffuseTexcoordAtlas.xy += material.Diffuse.AtlasNormOffset;
	NormalTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Normal.AtlasLayer);
	NormalTexcoordAtlas.xy *= vec2(material.Normal.AtlasNormSize);
	NormalTexcoordAtlas.xy += material.Normal.AtlasNormOffset;
	RoughnessTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Roughness.AtlasLayer);
	RoughnessTexcoordAtlas.xy *= vec2(material.Roughness.AtlasNormSize);
	RoughnessTexcoordAtlas.xy += material.Roughness.AtlasNormOffset;
	MetallicTexcoordAtlas = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.Metallic.AtlasLayer);
	MetallicTexcoordAtlas.xy *= vec2(material.Metallic.AtlasNormSize);
	MetallicTexcoordAtlas.xy += material.Metallic.AtlasNormOffset;
	TexcoordOrig = InTexcoord;
	Normal = InNormal;
	vec4 posLocal = vec4(InPositionLocal, 1.0);
	FragPosWorldSpace = vec3(instance.World * posLocal);
	DiffuseColor = material.DiffuseColor;

	mat3 normalMatrix = transpose(inverse(mat3(instance.World)));
	vec3 N = normalize(normalMatrix * Normal);
	vec3 T = normalize(normalMatrix * InTangent.xyz);
	vec3 B = normalize(cross(N, T)) * InTangent.w;
	TangentToWorld = mat3(T, B, N);

	Vertex_Passthrough(posLocal.xyz, instance, 0, gl_Position);
	Vertex_Func(posLocal.xyz, vec2(DiffuseTexcoordAtlas.xy), Normal, gl_InstanceID, instance, material, 0, gl_Position);
}