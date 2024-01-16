#version 430

layout(location = 0) out vec4 FragColor;

in vec3 Texcoord;
flat in float UserData;
flat in vec4 DiffuseColor;

uniform sampler2DArray TextureAtlas;

void main()
{
	if (UserData == 1.0) { discard; }

	FragColor = texture(TextureAtlas, Texcoord);
	FragColor *= DiffuseColor;
}