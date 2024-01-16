#version 430

layout(location = 0) out vec4 FragColor;

in vec2 Texcoord;
flat in vec4 GlyphInfo;
flat in vec4 GlyphAtlasInfos;
flat in vec4 DiffuseColor;

uniform sampler2D FontAtlas;

void main()
{
	vec4 pixel = texture(FontAtlas, Texcoord);
	if (pixel.x < 0.01) { discard; };
	FragColor = vec4(pixel.xxx, 1.0) * DiffuseColor;
}