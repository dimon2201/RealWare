#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_QUAD) || defined(RENDER_PATH_TRANSPARENT_COMPOSITE) || defined(RENDER_PATH_TEXT)
layout(location = 0) out vec4 FragColor;
#endif

#if defined(RENDER_PATH_TRANSPARENT)
layout(location = 0) out vec4 Accumulation;
layout(location = 1) out float Revealage;
#endif

in vec3 Texcoord;
flat in vec4 DiffuseColor;
#if defined(RENDER_PATH_TEXT)
flat in vec4 GlyphInfo;
flat in vec4 GlyphAtlasInfos;
#endif

#if defined(RENDER_PATH_OPAQUE) || defined(RENDER_PATH_TRANSPARENT)
uniform sampler2DArray TextureAtlas;
#endif

#if defined(RENDER_PATH_QUAD)
uniform sampler2D ColorTexture;
#endif

#if defined(RENDER_PATH_TRANSPARENT_COMPOSITE)
uniform sampler2D AccumulationTexture;
uniform sampler2D RevealageTexture;
#endif

#if defined(RENDER_PATH_TEXT)
uniform sampler2D FontAtlas;
#endif

void main()
{
	#if defined(RENDER_PATH_OPAQUE)
	FragColor = texture(TextureAtlas, Texcoord.xyz);
	if (Texcoord.z != -1.0) {
		FragColor *= DiffuseColor;
	} else {
		FragColor = DiffuseColor;
	}
	#endif
	
	#if defined(RENDER_PATH_TRANSPARENT)
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
	#endif
	
	#if defined(RENDER_PATH_TRANSPARENT_COMPOSITE)
	vec2 size = vec2(textureSize(AccumulationTexture, 0));

	vec4 accumulation = texelFetch(AccumulationTexture, ivec2(Texcoord.xy * size), 0);
	float revealage = texelFetch(RevealageTexture, ivec2(Texcoord.xy * size), 0).x;

	FragColor = vec4(accumulation.rgb / max(accumulation.a, 0.00001), 1.0 - revealage);
	#endif
	
	#if defined(RENDER_PATH_QUAD)
	vec2 size = vec2(textureSize(ColorTexture, 0));
	vec4 mainColor = texelFetch(ColorTexture, ivec2(Texcoord.xy * size), 0);
	FragColor = mainColor;
	#endif

	#if defined(RENDER_PATH_TEXT)
	vec4 pixel = texture(FontAtlas, Texcoord.xy);
	if (pixel.x < 0.01) { discard; };
	FragColor = vec4(pixel.xxx, 1.0) * DiffuseColor;
	#endif
}