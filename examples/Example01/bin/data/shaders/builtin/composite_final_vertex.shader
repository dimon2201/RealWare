out vec2 texcoord;

void main()
{
	vec3 positionLocalSpace = vec3(0.0);

	if (gl_VertexID == 0) 
	{
		positionLocalSpace = vec3(-1.0, -1.0, 0.0);
		texcoord = vec2(0.0, 0.0);
	}
	if (gl_VertexID == 1)
	{
		positionLocalSpace = vec3(-1.0, 1.0, 0.0);
		texcoord = vec2(0.0, 1.0);
	}
	if (gl_VertexID == 2)
	{
		positionLocalSpace = vec3(1.0, -1.0, 0.0);
		texcoord = vec2(1.0, 0.0);
	}
	if (gl_VertexID == 3)
	{
		positionLocalSpace = vec3(1.0, 1.0, 0.0);
		texcoord = vec2(1.0, 1.0);
	}

	gl_Position = vec4(positionLocalSpace, 1.0);
}