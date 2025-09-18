void Fragment_Func(in vec2 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor)
{
	TextureAtlasTexture tat = textureAtlasTextures[MyRedTexture];
	
	vec3 textureAtlasTexcoord = vec3(_texcoord.x, 1.0 - _texcoord.y, tat.TextureLayerInfo);
	textureAtlasTexcoord.xy *= vec2(tat.TextureInfo.zw);
	textureAtlasTexcoord.xy += tat.TextureInfo.xy;

	_fragColor = texture(TextureAtlas, textureAtlasTexcoord);

	//_fragColor = _textureColor * _materialDiffuseColor;
}