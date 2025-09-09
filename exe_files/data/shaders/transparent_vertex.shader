
void main()
{
	Instance instance = instances[gl_InstanceID];
	Material material = materials[instance.MaterialIndex];

	Texcoord = vec3(InTexcoord.x, 1.0 - InTexcoord.y, material.DiffuseTextureLayerInfo);
	Texcoord.xy *= vec2(material.DiffuseTextureInfo.zw);
	Texcoord.xy += material.DiffuseTextureInfo.xy;
	DiffuseColor = material.DiffuseColor;

	if (instance.Use2D == 0) {
	   gl_Position = ViewProjection * instance.World * vec4(InPositionLocal, 1.0);
	} else {
	   gl_Position = instance.World * vec4(InPositionLocal, 1.0);
	}
}