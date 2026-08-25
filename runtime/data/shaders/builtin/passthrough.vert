#version 460

void main()
{
    const vec3 position[] = {
        vec3(1.0, -1.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    };

    gl_Position = vec4(position[gl_VertexIndex], 1.0);
}