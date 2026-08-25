#version 460

out vec2 texcoord;

void main()
{
    const int vertexIndex = gl_VertexIndex;

    const vec3 positions[] = {
        vec3(1.0, -1.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(-1.0, -1.0, 0.0),
        vec3(-1.0, 1.0, 0.0)
    };

    const vec2 texcoords[] = {
        vec2(1.0, 0.0),
        vec2(1.0, 1.0),
        vec2(0.0, 0.0),
        vec2(0.0, 1.0)
    };

    texcoord = texcoords[vertexIndex];

    gl_Position = vec4(positions[vertexIndex], 1.0);
}