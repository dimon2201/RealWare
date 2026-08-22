layout(location = 0) out vec4 outPixelColor;

in vec2 texcoord;

uniform sampler2D colorTexture;

void main()
{
	vec2 texSize = vec2(textureSize(colorTexture, 0));
	vec4 color = texelFetch(colorTexture, ivec2(texcoord * texSize), 0);
	
	outPixelColor = color;
}