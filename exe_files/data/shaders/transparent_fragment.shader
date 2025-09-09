
void main()
{
	vec4 fragColor = texture(TextureAtlas, Texcoord);

	if (Texcoord.z != -1.0) {
	   fragColor *= DiffuseColor;
	} else {
	   fragColor = DiffuseColor;
	}

	float weight = clamp(pow(min(1.0, fragColor.a * 10.0) + 0.01, 3.0) * 1e8 *
	   pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);
	Accumulation = vec4(fragColor.rgb * fragColor.a, fragColor.a) * weight;
	Revealage = fragColor.a;
}