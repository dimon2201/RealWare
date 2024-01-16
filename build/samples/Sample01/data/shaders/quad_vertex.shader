#version 430

out vec2 Texcoord;

void main()
{
	vec3 PositionLocal = vec3(0.0);
	if (gl_VertexID == 0) { PositionLocal = vec3(-1.0, -1.0, 0.0); Texcoord = vec2(0.0, 0.0); }
	if (gl_VertexID == 1) { PositionLocal = vec3(-1.0, 1.0, 0.0); Texcoord = vec2(0.0, 1.0); }
	if (gl_VertexID == 2) { PositionLocal = vec3(1.0, -1.0, 0.0); Texcoord = vec2(1.0, 0.0); }
	if (gl_VertexID == 3) { PositionLocal = vec3(1.0, 1.0, 0.0); Texcoord = vec2(1.0, 1.0); }

	gl_Position = vec4(PositionLocal, 1.0);
}