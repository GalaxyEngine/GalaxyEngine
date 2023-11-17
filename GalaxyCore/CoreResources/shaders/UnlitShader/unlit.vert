#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;

out vec2 uv;
out vec3 normal;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    normal = aNor;
    uv = aTex;
}