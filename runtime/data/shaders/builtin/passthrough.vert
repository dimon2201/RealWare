#version 460

layout(location = 0) in vec3 inPositionLocalSpace;

void main()
{
    gl_Position = vec4(inPositionLocalSpace, 1.0);
}