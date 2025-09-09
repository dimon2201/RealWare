#version 430

layout(location = 0) out vec4 FragColor;

in vec2 Texcoord;

uniform sampler2D ColorTexture;
uniform sampler2D UIColorTexture;

void main()
{
	vec2 size = vec2(textureSize(ColorTexture, 0));

	vec4 mainColor = texelFetch(ColorTexture, ivec2(Texcoord * size), 0);
	vec4 uiColor = texelFetch(UIColorTexture, ivec2(Texcoord * size), 0);
	//if (dot(uiColor, uiColor) < 0.001) {
	//	FragColor = mainColor;
	//} else {
		FragColor = uiColor;
	//}
}