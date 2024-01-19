#version 430

layout(location = 0) out vec4 Accumulation;
layout(location = 1) out float Revealage;

in vec3 Texcoord;
flat in vec4 Color;

uniform sampler2DArray TextureAtlas;

void main()
{
	vec4 fragColor = texture(TextureAtlas, Texcoord);

	if (Texcoord.z != -1.0) {
	   fragColor *= Color;
	} else {
	   fragColor = Color;
	}

	float weight = clamp(pow(min(1.0, fragColor.a * 10.0) + 0.01, 3.0) * 1e8 *
	   pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	Accumulation = vec4(fragColor.rgb * fragColor.a, fragColor.a) * weight;
	Revealage = fragColor.a;
}