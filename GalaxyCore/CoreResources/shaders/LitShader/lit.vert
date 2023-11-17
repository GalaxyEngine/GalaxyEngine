#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;

out vec3 pos;  
out vec2 uv;
out vec3 normal;

uniform mat4 MVP;
uniform mat4 Model;

void main()
{
    gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
    pos = vec3(Model * vec4(aPos, 1.0f));
    normal = aNor;
    uv = aTex;
}