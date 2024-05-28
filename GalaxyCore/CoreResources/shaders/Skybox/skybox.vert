#version 450 core
layout (location = 0) in vec3 aPos;

out vec3 uv;

uniform mat4 VP;

void main()
{
    uv = aPos;
    gl_Position = VP * vec4(aPos, 1.0f);
}  