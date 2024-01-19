#version 430

layout(location = 0) out vec4 FragColor;

in vec3 Texcoord;
flat in vec4 Color;

uniform sampler2DArray TextureAtlas;

void main()
{
	FragColor = texture(TextureAtlas, Texcoord);
	
	if (Texcoord.z != -1.0) {
		FragColor *= Color;
	} else {
		FragColor = Color;
	}
}