void Fragment_Func(in vec3 _texcoord, in vec4 _textureColor, in vec4 _materialDiffuseColor, out vec4 _fragColor)
{
	_fragColor = _textureColor * _materialDiffuseColor;
}