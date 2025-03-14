#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTex;
layout (location = 2) in vec3 aNor;
layout (location = 3) in vec3 aTan;

out vec2 uv;
out vec3 viewDir; // Changed from viewPos to viewDir

uniform mat4 MVP;

void main()
{
  gl_Position = MVP * vec4(aPos, 1.0);
}