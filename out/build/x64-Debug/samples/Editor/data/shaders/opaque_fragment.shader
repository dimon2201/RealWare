#version 430

layout(location = 0) out vec4 FragColor;

struct Light
{
	vec4 Position;
	vec4 Color;
	vec4 DirectionAndScale;
	vec4 Attenuation;
};

layout(std430, binding = 2) buffer LightBuffer { uvec4 lightCount; Light lights[1024]; };

in vec3 Texcoord;
in vec4 WorldPosition;
flat in vec4 Color;
flat in uint LightCount;

uniform sampler2DArray TextureAtlas;

void main()
{
	float ambient = 0.25;
	vec4 colorFromLights = vec4(0.0);
	for (int i = 0; i < LightCount; i++)
	{
		vec3 lightColor = lights[i].Color.xyz;
		vec3 lightDirection = WorldPosition.xyz - lights[i].Position.xyz;
		float lightDistance = length(lightDirection);
		float attenuation = 
			lights[i].Attenuation.x +
			(lights[i].Attenuation.y * lightDistance) +
			(lights[i].Attenuation.z * lightDistance * lightDistance);
		
		colorFromLights += vec4(lightColor / attenuation, 0.0);
	}

	FragColor = colorFromLights + (ambient * texture(TextureAtlas, Texcoord));
	
	if (Texcoord.z != -1.0) {
		FragColor *= Color;
	} else {
		FragColor = Color;
	}
}