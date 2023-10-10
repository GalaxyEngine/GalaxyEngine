#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;

out vec2 uv;

void main()
{
    gl_Position = vec4(aPos.xy, 0.0, 1.0); 
    uv = vec2(aTex.x, 1 - aTex.y);
}  