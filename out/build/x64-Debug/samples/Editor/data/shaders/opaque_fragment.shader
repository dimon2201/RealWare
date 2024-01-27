#version 430

layout(location = 0) out vec4 FragColor;

struct Light
{
	vec4 Position;
	vec4 Color;
	vec4 DirectionAndScale;
};

layout(std430, binding = 2) buffer LightBuffer { uvec4 lightCount; Light lights[1024]; };

in vec3 Texcoord;
flat in vec4 Color;
flat in uint LightCount;

uniform sampler2DArray TextureAtlas;

void main()
{
	vec4 colorFromLights = vec4(0.0);
	for (int i = 0; i < LightCount; i++)
	{
		colorFromLights += lights[i].Color * 0.5;
	}

	FragColor = colorFromLights + texture(TextureAtlas, Texcoord);
	
	if (Texcoord.z != -1.0) {
		FragColor *= Color;
	} else {
		FragColor = Color;
	}
}