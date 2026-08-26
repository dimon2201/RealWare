#version 460

layout(location = 0) out vec3 color;

void main()
{
    const int vertexIndex = gl_VertexIndex;

    const vec3 position[] = {
        vec3(1.0, -1.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    };

    const vec3 colors[] = {
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    };

    color = colors[vertexIndex];

    gl_Position = vec4(position[vertexIndex], 1.0);
}